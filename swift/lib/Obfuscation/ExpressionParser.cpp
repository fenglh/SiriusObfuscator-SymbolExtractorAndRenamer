#include "swift/Obfuscation/ExpressionParser.h"
#include "swift/Obfuscation/ParameterDeclarationParser.h"
#include "swift/Obfuscation/NominalTypeDeclarationParser.h"
#include "swift/Obfuscation/Utils.h"
#include "swift/Obfuscation/DeclarationParsingUtils.h"

namespace swift {
namespace obfuscation {
  
llvm::Expected<AbstractFunctionDecl*>
declarationOfFunctionCalledInExpression(CallExpr *CallExpression) {
  auto *CallFn = CallExpression->getFn();

  if (auto *DotSyntaxCallExpression = dyn_cast<DotSyntaxCallExpr>(CallFn)) {
    auto *DotFn = DotSyntaxCallExpression->getFn();
    
    if (auto *OtherConstructor = dyn_cast<OtherConstructorDeclRefExpr>(DotFn)) {
      // It's a super call like super.init()
      auto *Decl = OtherConstructor->getDecl();

      if (auto *FunctionDeclaration =
            dyn_cast_or_null<AbstractFunctionDecl>(Decl)) {
        return FunctionDeclaration;
      }
    } else {
      // It's not a super.init call, just a function call
      if (auto *DeclRefExpression = dyn_cast<DeclRefExpr>(DotFn)) {
        auto *Decl = DeclRefExpression->getDecl();

        if (auto *FunctionDeclaration =
              dyn_cast_or_null<AbstractFunctionDecl>(Decl)) {
          return FunctionDeclaration;
        }
      }
    }
  } else if (auto *Constructor = dyn_cast<ConstructorRefCallExpr>(CallFn)) {
    // It's a constructor call
    auto * ConstructorFn = Constructor->getFn();

    if (auto *DeclarationRefExpression = dyn_cast<DeclRefExpr>(ConstructorFn)) {
      auto* Decl = DeclarationRefExpression->getDecl();

      if (auto *FunctionDeclaration =
            dyn_cast_or_null<AbstractFunctionDecl>(Decl)) {
        return FunctionDeclaration;
      }
    }
  } else if (auto *Expression = dyn_cast<Expr>(CallFn)) {
    // This branch is executed for example when
    // a function is being called inside set {} block
    auto *Decl = Expression->getReferencedDecl().getDecl();
    
    if (auto *FunctionDeclaration =
          dyn_cast_or_null<AbstractFunctionDecl>(Decl)) {
      return FunctionDeclaration;
    }
  }
  return stringError("Cannot found supported Call Expression subtree pattern");
}
  
std::vector<std::pair<Identifier, SourceLoc>>
validArguments(CallExpr *CallExpression) {

  std::vector<std::pair<Identifier, SourceLoc>> ValidArguments;
  
  for (unsigned i = 0; i < CallExpression->getNumArguments(); ++i) {
    auto Label = CallExpression->getArgumentLabels()[i];
    auto Location = CallExpression->getArgumentLabelLoc(i);
    if (Location.isValid()) {
      ValidArguments.push_back(std::make_pair(Label, Location));
    }
  }
  
  return ValidArguments;
}
  
SymbolsOrError parseCallExpressionWithArguments(CallExpr* CallExpression) {
  
  std::vector<SymbolWithRange> Symbols;
  auto FunctionDeclarationOrError =
    declarationOfFunctionCalledInExpression(CallExpression);
  if (auto Error = FunctionDeclarationOrError.takeError()) {
    return std::move(Error);
  }
  auto FunctionDeclaration = FunctionDeclarationOrError.get();
  
  auto SymbolsOrError =
    parseFunctionFromCallExpressionForParameters(FunctionDeclaration);
  if (auto Error = SymbolsOrError.takeError()) {
    return std::move(Error);
  }

  auto CopyOfSymbols = SymbolsOrError.get();

  auto ValidArguments = validArguments(CallExpression);
  for (size_t i = 0; i < ValidArguments.size(); ++i) {

    auto Label = ValidArguments[i].first;
    auto Location = ValidArguments[i].second;
    if (ValidArguments.size() == SymbolsOrError.get().size()) {
      // The same number of named arguments in call and
      // external/single parameters in function means that
      // there are no parameters in this function that are default
      // or with the external name
      auto Symbol = SymbolsOrError.get()[i];
      if (Location.isValid() && Symbol.Symbol.Name == Label.str().str()) {
        auto Range = CharSourceRange(Location, Label.getLength());
        Symbols.push_back(SymbolWithRange(Symbol.Symbol, Range));
      }

    } else {
      // There is different number of named arguments in call
      // and external/single parameters in function. It means that
      // some of the parameters are not required
      // (default or without external name)
      for (auto Symbol : CopyOfSymbols) {
        if (Location.isValid() && Symbol.Symbol.Name == Label.str().str()) {
          removeFromVector(CopyOfSymbols, Symbol);
          auto Range = CharSourceRange(Location, Label.getLength());
          Symbols.push_back(SymbolWithRange(Symbol.Symbol, Range));
          break;
        }
      }
    }
  }

  return Symbols;
}
  
SymbolsOrError
parseGenericParameters(BoundGenericType *BoundGenericType,
                       SourceLoc OpeningAngleBracketLoc) {
  std::vector<SymbolWithRange> Symbols;
  auto Parameters = BoundGenericType->getGenericArgs();
  for (auto Parameter : Parameters) {
    NominalTypeDecl *ParameterDecl = nullptr;
    if (OptionalType::classof(Parameter.getPointer())) {
      ParameterDecl =
      Parameter->getOptionalObjectType()->getAnyNominal();
    } else {
      ParameterDecl = Parameter->getAnyNominal();
    }
    auto ParameterSymbol = parse(ParameterDecl);
    if (auto Error = ParameterSymbol.takeError()) {
      return std::move(Error);
    }
    auto ParameterName =
    ParameterDecl->getBaseName().getIdentifier().str();
    auto GenericArgRange =
      rangeOfFirstOccurenceOfStringInSourceLoc(ParameterName,
                                               OpeningAngleBracketLoc);
    if (auto Error = GenericArgRange.takeError()) {
      return std::move(Error);
    }
    Symbols.push_back(SymbolWithRange(ParameterSymbol.get(),
                                      GenericArgRange.get()));
  }
  return Symbols;
}

SymbolsOrError parse(CallExpr* CallExpression) {
  if (CallExpression->hasArgumentLabelLocs()) {
    return parseCallExpressionWithArguments(CallExpression);
  }
  return stringError("Unsupported type of expression");
}
  
// This function handles the specific case of `is` cast of non-optional
// to optional type or vice versa. In contrast to other castings
// (represented as is_subtype_expr), this kind of cast is represented
// in AST as enum_is_case_expr node. When parsing this kind of cast
// we don't get the callback in SymbolsWalkerAndCollector with NominalTypeDecl
// representing the CastType (cast-to type) so we have to extract it
// from the EnumIsCaseExpression.
SymbolsOrError parse(EnumIsCaseExpr* EnumIsCaseExpression) {
  ExplicitCastExpr *ExplicitCastExpression = nullptr;
  
  // This callback invoked using forEachChildExpr() is used to extract the
  // declaration of the CastType and the location of the `is` keyword.
  const std::function<Expr*(Expr*)> &callback =
    [&ExplicitCastExpression](Expr* Child) -> Expr* {
      
    // We're looking for CoerceExpr (non-optional to optional type cast)
    // or ConditionalCheckedCastExpr (optional to non-optional type cast)
    // which both are subclasses of ExplicitCastExpr.
    if (ExplicitCastExpr::classof(Child)) {
      ExplicitCastExpression = dyn_cast<ExplicitCastExpr>(Child);
    }
      
    return Child;
  };
  EnumIsCaseExpression->forEachChildExpr(callback);
  
  if (ExplicitCastExpression != nullptr) {
    auto CastType = ExplicitCastExpression->getCastTypeLoc().getType();
    
    // The data representing the location of the CastType in the expression
    // seems to be impossible to retrieve from the EnumIsCastExpression
    // and its subexpressions. We have to calculate the CastType location
    // later using `is` keyword and CastType name.
    auto IsKeywordSourceLoc = ExplicitCastExpression->getAsLoc();
  
    Type UnwrappedCastType;
    if (ConditionalCheckedCastExpr::classof(ExplicitCastExpression)) {
      UnwrappedCastType = CastType;
    } else if (CoerceExpr::classof(ExplicitCastExpression)) {
      UnwrappedCastType = CastType->getOptionalObjectType();
    } else {
      return stringError("Unsupported type of explicit cast expression");
    }
    
    NominalTypeDecl *CastTypeDeclaration = UnwrappedCastType->getAnyNominal();
    
    if (CastTypeDeclaration != nullptr) {
      auto CastTypeSymbol = parse(CastTypeDeclaration);
      if (auto Error = CastTypeSymbol.takeError()) {
        return std::move(Error);
      }
      
      auto CastTypeName =
        CastTypeDeclaration->getBaseName().getIdentifier().str();
      auto CastTypeRange =
        rangeOfFirstOccurenceOfStringInSourceLoc(CastTypeName,
                                                 IsKeywordSourceLoc);
      if (auto Error = CastTypeRange.takeError()) {
        return std::move(Error);
      }
      
      std::vector<SymbolWithRange> Symbols;
      Symbols.push_back(SymbolWithRange(CastTypeSymbol.get(),
                                        CastTypeRange.get()));
      
      if (auto *GenericBoundType =
            dyn_cast<BoundGenericType>(UnwrappedCastType.getPointer())) {
        
        auto GenericNameEndLoc = CastTypeRange.get().getEnd();
        auto OpeningAngleBracketRange =
          rangeOfFirstOccurenceOfStringInSourceLoc("<", GenericNameEndLoc);
        if (auto Error = OpeningAngleBracketRange.takeError()) {
          return std::move(Error);
        }
        auto OpeningAngleBracketLoc = OpeningAngleBracketRange.get().getStart();
        
        auto GenericParamsSymbols =
          parseGenericParameters(GenericBoundType, OpeningAngleBracketLoc);
        if (auto Error = GenericParamsSymbols.takeError()) {
          return std::move(Error);
        }
        
        copyToVector(GenericParamsSymbols.get(), Symbols);
      }
      
      return Symbols;
    }
  }
  
  return stringError("Failed to extract the cast-to type symbol"
                     "from the EnumIsCase expression");
}

SymbolsOrError extractSymbol(Expr* Expression) {
  if (auto *CallExpression = dyn_cast<CallExpr>(Expression)) {
    return parse(CallExpression);
  } else if (auto *EnumIsCaseExpression = dyn_cast<EnumIsCaseExpr>(Expression)) {
    // Expression of `is` casting non-optional to optional type or vice versa.
    return parse(EnumIsCaseExpression);
  }
  
  return stringError("Unsupported type of expression");
}
  
} //namespace obfuscation
} //namespace swift
