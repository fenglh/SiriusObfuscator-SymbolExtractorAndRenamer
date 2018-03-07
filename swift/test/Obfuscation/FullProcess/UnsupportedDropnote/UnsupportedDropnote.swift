//XFAIL: *
//RUN: %target-prepare-obfuscation-for-file "UnsupportedDropnote" %target-run-full-obfuscation


import AppKit

class Test {
  func testFunc() {}
}

// debug blocks are omitted
final class DebugBlock {
  fileprivate init() {
    #if !DEBUG
      let testInDebug = Test()
    #endif
  }
}

// for each stuff

final class ForEachController: NSViewController {
  
  var unitsSegmentedControl: NSSegmentedControl!
  
  var titles: [String] = []
  
  fileprivate func buggyFunc() {
    titles.enumerated().map {
      index, title in (title, index)
      }.forEach(unitsSegmentedControl.setLabel(_:forSegment:))
    
    unitsSegmentedControl.accessibilityHint = ""
  }
}

// mocking trick

protocol KeyValueStoreType {
  func object(forKey defaultName: String) -> Any?
  func set(_ value: Any?, forKey defaultName: String)
  func removeObject(forKey defaultName: String)
  func synchronize() -> Bool
}

extension UserDefaults: KeyValueStoreType {
  
}


