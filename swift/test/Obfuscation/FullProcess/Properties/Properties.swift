//RUN: %target-prepare-obfuscation-for-file "Properties" %target-run-full-obfuscation

import AppKit

struct DummyStruct {}

// stored let and var properties of different types
class StoredClass {
  let letInt: Int = 0
  var varInt: Int
  let letString: String = "0"
  var varString: String
  let letStruct: DummyStruct = DummyStruct()
  var varStruct: DummyStruct
}

struct StoredStruct {
  let letInt: Int = 0
  var varInt: Int
  let letString: String = "0"
  var varString: String
  let letStruct: DummyStruct = DummyStruct()
  var varStruct: DummyStruct
}

// computed properties of different types
protocol ComputedProtocol {
  var varInt: Int { get }
  var varString: String { get set }
  var varStruct: DummyStruct { get set }
}

class ComputedClass: ComputedProtocol {
  var varInt: Int { return 42 }
  var varString: String { get { return "42" } set {  } }
  var varStruct = DummyStruct()
}

struct ComputedStruct {
  var varInt: Int { return 42 }
  var varString: String { get { return "42" } set {  } }
  var varStruct: DummyStruct = DummyStruct()
}

//computed properties required by other modules
class OtherStruct: NSValidatedUserInterfaceItem {
  var action: Selector? { return nil }
  var tag: Int = 0
}

// stored properties required by other modules
class ViewClass: NSView {
  override var subviews: [NSView] { get { return [] } set { } }
  override var window: NSWindow? { return nil }
}

// properties with generic parameters
class GenericUsingClass {
  let array: Array<Int> = []
  let map: [String : Int] = [:]
}

// properties usage
class PropertiesUsingClass {
  var array: Array<Int> = []
  var map: [String : Int] = [:]

  func foo() {
    array = [42]
    map["42"] = array[0]
  }
}

// implicit error name in catch block should not be renamed
func canThrowErrors() throws {}
func a() {
  do {
    try canThrowErrors()
  } catch {
    error
  }
}

// implicit variable name inside setter in catch block should not be renamed
struct ImplicitSetter {
  var foo: String {
    get {
      return "foo"
    }
    set {
      newValue
    }
  }
}