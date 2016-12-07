//
//  RootViewController.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/06/30.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import UIKit

// class RootViewController: UINavigationController {
class RootViewController: UIViewController {

    var model_     : Model?    = nil
    
    var isLoading  : Bool      = false
    
    var camVC : CamViewController? = nil

    func initModel() -> Model? {
        
        // model loading
        if let modelFileURL = ModelData.defaultModelFileURL() {
            if NSFileManager.defaultManager().fileExistsAtPath(modelFileURL.path!) {
                model_ = Model()
                model_!.load(modelFileURL.path!)
            }
        }
        
        return model_
    }
    
    func enableLoadingView(state:Bool) {
        if isLoading == state {
            return
        }
        
        if state {
            isLoading = state
            let storyboard = UIStoryboard(name:"Main", bundle:nil)
            
            let vc = storyboard.instantiateViewControllerWithIdentifier("ModelLoadingView") as! ModelLoadingViewController
            self.addViewController(vc, animated:true)
        } else {
            isLoading = state
            self.closeChildViewController(true)
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // check model file
        if ModelData.isExistDefaultModelFile() {
            self.loadingModelData(ModelData.selectedModelDataIndex, animated:true)
        } else {
            var selectedIndex = 0
            // find default download target
            for i in 0 ..< ModelData.numOfModels() {
                if ModelData.isExistModelFile(i) != true {
                    // set download target index
                    if ModelData.isExecModelDataSize(i) {
                        selectedIndex = i
                        break
                    }
                }
            }

            self.showDownloadView(true, selectedIndex: selectedIndex)
        }
    }
    

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


    func createCamViewController() -> UIViewController {
        let storyboard = UIStoryboard(name:"Main", bundle:nil)
    
        let vc = storyboard.instantiateViewControllerWithIdentifier("CamViewController") as! CamViewController
        vc.model_ = self.model_
        return vc
    }
    
    
    func addViewController(vc: UIViewController, animated:Bool) {
        
        let viewFrame = self.view.frame
        
        self.addChildViewController(vc)

        vc.didMoveToParentViewController(self)
        vc.view.frame = viewFrame
        
        self.view.addSubview(vc.view)
        
        if animated {
            UIApplication.sharedApplication().beginIgnoringInteractionEvents()
            UIView.animateWithDuration(0.3, delay: 0, options: .CurveEaseOut, animations: {
                // animation
                
            }, completion: { (finished: Bool) in
                UIApplication.sharedApplication().endIgnoringInteractionEvents()
            })
        } else {
        }
    }
    
    func closeChildViewController(animated:Bool, completion:(() -> Void)? = nil) {
        
        let vc = self.childViewControllers.last!
        
        if (animated) {
            UIApplication.sharedApplication().beginIgnoringInteractionEvents()
            UIView.animateWithDuration(0.3, delay: 0, options: .CurveEaseOut, animations: {
                
            },
            completion: { (finished: Bool) in
                UIApplication.sharedApplication().endIgnoringInteractionEvents()

                vc.view.removeFromSuperview()
                vc.willMoveToParentViewController(nil)
                vc.removeFromParentViewController()
                completion?()
            })

        } else {
            vc.view.removeFromSuperview()
            vc.willMoveToParentViewController(nil)
            vc.removeFromParentViewController()
            completion?()
        }
    }


    func isNeedToLoadModelData(index: Int) -> Bool {
        if model_ == nil {
            return true
        }
        
        if ModelData.selectedModelDataIndex != index {
            return true
        }
        
        return false
    }
    
    
    func loadingModelData(index: Int, animated:Bool) {
        guard self.isNeedToLoadModelData(index) else {
            return
        }
        
        // no camVC or camVC is top of childViewController
        if camVC != nil {
            let lastVC = self.childViewControllers.last
            if lastVC != camVC {
                return
            }
        }
        
        ModelData.selectedModelDataIndex = index
        
        self.enableLoadingView(true)
        
        // load model in different thread
        let queue = dispatch_queue_create("LoadingModelData", DISPATCH_QUEUE_CONCURRENT)
        
        dispatch_async(queue) {
            
            self.initModel()
            
            dispatch_async(dispatch_get_main_queue()) {
                self.enableLoadingView(false)
                
                // show CamVC
                if self.camVC == nil {
                    self.showCamView(animated)
                } else {
                    self.camVC!.model_ = self.model_
                }
            }
        }
    }


    func showCamView(animated: Bool) {

        let storyboard = UIStoryboard(name:"Main", bundle:nil)
        
        camVC = storyboard.instantiateViewControllerWithIdentifier("CamViewController") as? CamViewController
        camVC!.model_ = self.model_

        self.addViewController(camVC!, animated:animated)
    }
    
    
    func showDownloadView(animated: Bool, selectedIndex: Int) {
        let storyboard = UIStoryboard(name:"Main", bundle:nil)
        
        let vc = storyboard.instantiateViewControllerWithIdentifier("DownloadViewController") as! DownloadViewController

        vc.selectedIndex = selectedIndex
        
        self.addViewController(vc, animated:animated)
    }
    
    func showSettingsView(animated: Bool) {
        let storyboard = UIStoryboard(name:"Main", bundle:nil)
        
        let vc = storyboard.instantiateViewControllerWithIdentifier("SettingsViewController")
        
        self.addViewController(vc, animated:animated)
    }
}
