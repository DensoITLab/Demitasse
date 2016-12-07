//
//  ModelLoadingViewController.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/07/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import UIKit

class ModelLoadingViewController: UIViewController {

    @IBOutlet weak var activityView : UIActivityIndicatorView!

    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        
        activityView.startAnimating()
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        
        activityView.stopAnimating()
    }
}
