#ifndef SourceFileWalker_h
#define SourceFileWalker_h

#include "swift/Frontend/Frontend.h"
#include "swift/Obfuscation/DataStructures.h"

#include <set>

namespace swift {
namespace obfuscation {

/// Identifies the symbols in the given Swift AST SourceFile object with
/// the index of their occurence among all files.
///
/// The SourceFile might be extracted from the compiler after it consumes and
/// performs the semantic analysis of the Swift cource code file. It's an AST
/// tree that contains all the AST nodes found in the source file.
///
/// This method walks through the SourceFile tree and collects all the symbols
/// that are of our interest and the place they occur in the set of
/// SymbolWithRange structures.
///
/// Typical usage:
/// \code
/// for (auto* File : CompilerInstance.getMainModule()->getFiles()) {
///   if (auto* SourceFile = dyn_cast<SourceFile>(File)) {
///     auto SymbolsWithRanges = walkAndCollectSymbols(*SourceFile);
/// \endcode
///
/// \param SourceFile AST tree representing the Swift source file.
///
/// \returns the occurrences of symbols with their metadata, place
///          and occurrence index.
std::set<IndexedSymbolWithRange, IndexedSymbolWithRange::SymbolWithRangeCompare>
  walkAndCollectSymbols(SourceFile &SourceFile);

} //namespace obfuscation
} //namespace swift

#endif /* SourceFileWalker_h */
