//
//  ModelData.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/07/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import Foundation

class ModelData {
    
    private enum SettingsKey : String {
        case SelectedModelDataID = "selectedModelDataID"
    }
    
    private var models: [NSDictionary]? = nil
    private var selectedIndex       : Int    = -1
    private var selectedModelName   : String = "none"
    private var maxMBMemory         : Int    = 1024
    
    private var userDefaults : NSUserDefaults!

    private static let sharedInstance = ModelData()


    // set default model index
    private func defaultModelIndex() {
        let defaultModelName : String
        if maxMBMemory >= 1024 {
            defaultModelName = "vgg_16.model"
        } else {
            defaultModelName = "vgg_cnn_f.model"
        }
        
        for (i, item) in self.models!.enumerate() {
            let aModelID = item["model"] as! String
            if aModelID == defaultModelName {
                selectedIndex     = i
                selectedModelName = aModelID
            }
        }
    }

    //
    private func findDownloadedModel(modelName:String?) -> Bool {
        for (i, item) in self.models!.enumerate() {
            let aModelID = item["model"] as! String
            
            if modelName != nil {
                if aModelID != modelName! {
                    // skip it
                    continue
                }
            }
            
            let url = ModelData.modelFilePathURL(aModelID)
            let isExist = NSFileManager.defaultManager().fileExistsAtPath(url.path!)
            
            if isExist {
                let dataSize = (item["size"] as! NSNumber).integerValue * 2
                if dataSize <= maxMBMemory {
                    selectedIndex     = i
                    selectedModelName = aModelID
                    return true
                }
            }
        }
        
        return false
    }

    init() {
        // get host info
        if let info = ModelData.getHostBasicInfo() {
            // device mem is 1GB
            maxMBMemory = Int(info.max_mem / (1024 * 1024))
        }

        let path : String = NSBundle.mainBundle().pathForResource("model_data", ofType: "plist")!
        models = NSArray(contentsOfFile: path) as? [NSDictionary]
        
        
        userDefaults = NSUserDefaults.standardUserDefaults()
        
        if let modelID = userDefaults.valueForKey(SettingsKey.SelectedModelDataID.rawValue) as? String {
            if self.findDownloadedModel(modelID) {
                // selected model found
                return
            }
        }
        
        if self.findDownloadedModel(nil) {
            // downloaded model found
            return
        }
        
        // set default model index
        self.defaultModelIndex()
    }
    

    class var maxDeviceMemory : Int {
        return sharedInstance.maxMBMemory
    }

    class func getHostBasicInfo() -> host_basic_info? {
        let hostPort = mach_host_self()
        
        let ptr      = UnsafeMutablePointer<host_basic_info>.alloc(sizeof(host_basic_info))
        var hostBasicInfo = ptr.memory
        ptr.destroy()

        
        var count = mach_msg_type_number_t(
            sizeof(host_basic_info) / sizeof(integer_t)
        )
        let err = withUnsafePointer(&hostBasicInfo) {
            host_info(hostPort, HOST_BASIC_INFO, host_info_t($0), &count)
        }
        return err == 0 ? hostBasicInfo : nil
    }
    
    
    class func numOfModels() -> Int {

        guard let aModels = sharedInstance.models else {
            return 0
        }

        return aModels.count
    }
    

    class func modelName(index: Int) -> String? {
        guard let item = sharedInstance.models?[index] else {
            return nil
        }
        
        let name = item["name"] as! String
        return name
    }
   

    private func modelDataSize(index: Int) -> Int {
        guard let item = self.models?[index] else {
            return 0
        }
        
        let mb = item["size"] as! NSNumber
        return mb.integerValue
    }
    
    class func modelDataSize(index: Int) -> Int {
        return sharedInstance.modelDataSize(index)
    }
    

    class func isExecModelDataSize(index: Int) -> Bool {
        guard let item = sharedInstance.models?[index] else {
            return false
        }
        
        let mb = item["size"] as! NSNumber
        return (sharedInstance.maxMBMemory >= mb.integerValue * 2)
    }


    class func modelFile(index: Int) -> String? {
        guard let item = sharedInstance.models?[index] else {
            return nil
        }
        
        let name = item["model"] as! String
        return name
    }

    class func modelFileURL(index: Int) -> NSURL? {
        guard let _ = sharedInstance.models?[index] else {
            return nil
        }
        
        guard let name = ModelData.modelFile(index) else {
            return nil
        }
        
        return ModelData.modelFilePathURL(name)
    }
    
    class func defaultModelFileURL() -> NSURL? {
        return ModelData.modelFileURL(sharedInstance.selectedIndex)
    }
    
    class func isExistModelFile(index: Int) -> Bool {
        guard let url = ModelData.modelFileURL(index) else {
            return false
        }
        
        let isExist = NSFileManager.defaultManager().fileExistsAtPath(url.path!)
        
        return isExist
    }

    class func isExistDefaultModelFile() -> Bool {
        return ModelData.isExistModelFile(sharedInstance.selectedIndex)
    }
    
    class func modelFileDownloadURL(index: Int) -> NSURL? {
        guard let info = sharedInstance.models?[index] else {
            return nil
        }
        
        guard let url = info["url"] as? String else {
            return nil
        }
        
        return NSURL(string:url)
    }

    class var selectedModelDataIndex : Int {
        get {
            return sharedInstance.selectedIndex
        }
        
        set (newValue) {
            guard newValue >= 0 && newValue < numOfModels() else {
                return
            }
            
            let item = sharedInstance.models![newValue]
            let modelID = item["model"] as! String
            
            sharedInstance.selectedIndex        = newValue
            sharedInstance.selectedModelName    = modelID
            
            let userDefaults = sharedInstance.userDefaults
            userDefaults.setValue(modelID,
                                  forKey:SettingsKey.SelectedModelDataID.rawValue)
            userDefaults.synchronize()
        }
    }

    class func modelFilePathURL(name: String) -> NSURL {
        let path = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.DocumentDirectory, NSSearchPathDomainMask.UserDomainMask, true)
        let documentDirectoryPath: NSString = path[0]
        
        let filePath = documentDirectoryPath.stringByAppendingPathComponent(name)
        
        let url = NSURL(fileURLWithPath: filePath)
        
        return url
    }
    
    class var selectedModelName : String {
        return sharedInstance.selectedModelName
    }
}