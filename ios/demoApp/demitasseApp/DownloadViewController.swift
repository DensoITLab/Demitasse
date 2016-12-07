//
//  DownloadViewController.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/06/30.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import UIKit


let model_file_download_path = "https://s3-ap-northeast-1.amazonaws.com/ditlab-public"

class DownloadViewController: UIViewController, UITableViewDataSource, UITableViewDelegate, NSURLSessionDelegate {
    
    @IBOutlet weak var tableView        : UITableView!
    
    @IBOutlet weak var execButton       : UIButton!
    @IBOutlet weak var progressBar      : UIProgressView!
    @IBOutlet weak var progressLabel    : UILabel!
    @IBOutlet weak var noticeLabel      : UILabel!

    var downloadTask: NSURLSessionDownloadTask? = nil
    var backgroundSession: NSURLSession?        = nil

    var isDownloading : Bool = false

    var selectedIndex : Int  = 0
    
    func prepareDownloadingView() {
        if isDownloading {
            execButton.setTitle(NSLocalizedString("Cancel", comment:""), forState: UIControlState.Normal)
            execButton.backgroundColor = UIColor.grayColor()
            
            progressBar.hidden   = false
            progressLabel.hidden = false
            
            tableView.userInteractionEnabled = false
        } else {
            execButton.setTitle(NSLocalizedString("Download", comment:""), forState: UIControlState.Normal)
            execButton.backgroundColor = execButton.tintColor
            progressBar.hidden   = true
            progressLabel.hidden = true
            progressBar.progress = 0
            progressLabel.text   = "0 MB"
            
            tableView.userInteractionEnabled = true
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        self.prepareDownloadingView()
        
        if ModelData.maxDeviceMemory >= 2048 {
            noticeLabel.hidden = true
        } else {
            noticeLabel.hidden = false
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        
        if isDownloading {
            downloadTask?.cancel()
            backgroundSession?.invalidateAndCancel()
        }
    }

    // MARK: UITableViewDataSource
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return ModelData.numOfModels()
    }
    
    func tableView(tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return NSLocalizedString("Pretrained Model Data", comment:"")
    }
    
    func tableView(tableView: UITableView, willDisplayHeaderView view: UIView, forSection section: Int) {
        if let headerView = view as? UITableViewHeaderFooterView {
            headerView.backgroundView?.backgroundColor = UIColor.whiteColor()
            headerView.textLabel!.textColor = UIColor(red: 63/255.0, green: 28/255.0, blue: 0/255.0, alpha: 1.0)
        }
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("Cell", forIndexPath:indexPath)
        
        self.configureCell(cell, atIndexPath:indexPath)
        
        return cell
    }

    func configureCell(cell: UITableViewCell, atIndexPath indexPath:NSIndexPath) {

        var state : String
        
        if ModelData.isExistModelFile(indexPath.row) {
            state = NSLocalizedString("Downloaded", comment:"")
            cell.accessoryType = .None
        } else {
            state = NSLocalizedString("Not Downloaded", comment:"")
            if indexPath.row == selectedIndex {
                cell.accessoryType = .Checkmark
            } else {
                cell.accessoryType = .None
            }
        }
        
        if ModelData.isExecModelDataSize(indexPath.row) {
            cell.textLabel!.textColor = UIColor(red: 63/255.0, green: 28/255.0, blue: 0/255.0, alpha: 1.0)
        } else {
            cell.textLabel!.textColor = UIColor.lightGrayColor()
            state = NSLocalizedString("Not Supported", comment:"")
        }
        
        cell.textLabel!.text       = ModelData.modelName(indexPath.row)
        cell.detailTextLabel!.text = String(format:"%d MB %@", ModelData.modelDataSize(indexPath.row), state)
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {

        // reset selection
        tableView.deselectRowAtIndexPath(indexPath, animated: true)

        if ModelData.isExecModelDataSize(indexPath.row) == false {
            return
        }

        if ModelData.isExistModelFile(indexPath.row) {
            return
        }
        
        selectedIndex = indexPath.row

        tableView.reloadRowsAtIndexPaths(tableView.indexPathsForVisibleRows!, withRowAnimation: UITableViewRowAnimation.Fade)
    }
    
    @IBAction func onStartDownload(sender: AnyObject) {
        
        if isDownloading != true {
            if ModelData.isExistModelFile(selectedIndex) {
                return
            }

            guard let url = ModelData.modelFileDownloadURL(selectedIndex) else {
                return
            }

            isDownloading = !isDownloading

            let backgroundSessionConfiguration = NSURLSessionConfiguration.backgroundSessionConfigurationWithIdentifier("backgroundTask")
            
            backgroundSession = NSURLSession(configuration: backgroundSessionConfiguration, delegate: self, delegateQueue: NSOperationQueue.mainQueue())
            
            downloadTask = backgroundSession!.downloadTaskWithURL(url)
            downloadTask!.resume()
        } else {
            isDownloading = !isDownloading

            downloadTask?.cancel()
            
            backgroundSession?.invalidateAndCancel()
            
            if let rootVC = self.parentViewController as? RootViewController {
                if rootVC.childViewControllers.count > 1 {
                    rootVC.closeChildViewController(true, completion: nil)
                }
            }
        }
        self.prepareDownloadingView()
    }
    

    func downloadCompleted(path: String) {
        let fileManager = NSFileManager.defaultManager()
        if fileManager.fileExistsAtPath(path) {
            if let rootVC = self.parentViewController as? RootViewController {
                rootVC.closeChildViewController(true, completion: {
                    
                    NSNotificationCenter.defaultCenter().postNotificationName("updateModelData", object: nil)
                    rootVC.loadingModelData(self.selectedIndex, animated: true)
                })
            }
        }
    }

    
    // MARK: NSURLSessionDelegate
    func URLSession(session: NSURLSession,
                    downloadTask: NSURLSessionDownloadTask,
                    didFinishDownloadingToURL location: NSURL) {
        
        // create model file path
        let requestURL   = downloadTask.currentRequest!.URL!
        
        let fileName     = requestURL.lastPathComponent!
        let modelFileURL = ModelData.modelFilePathURL(fileName)
        
        // let modelFileURL = ModelData.modelFileURL(selectedIndex)!

        let fileManager = NSFileManager.defaultManager()
        if fileManager.fileExistsAtPath(modelFileURL.path!) {
            do {
                try fileManager.removeItemAtPath(modelFileURL.path!)
            } catch {
                
            }
        }

        do {
            try fileManager.moveItemAtURL(location, toURL: modelFileURL)
            downloadCompleted(modelFileURL.path!)
        } catch {
            print("An error occurred while moving file to destination url")
        }
        
        isDownloading = false
        self.prepareDownloadingView()
    }

    func URLSession(session: NSURLSession,
                       task: NSURLSessionTask,
                       didCompleteWithError error: NSError?) {
        if error == nil  {
            // success
            session.finishTasksAndInvalidate()
            return
        }
        
        // create model file path
        let modelFileURL = ModelData.modelFileURL(selectedIndex)!
        
        let fileManager = NSFileManager.defaultManager()
        if fileManager.fileExistsAtPath(modelFileURL.path!) {
            do {
                try fileManager.removeItemAtPath(modelFileURL.path!)
            } catch {
                
            }
        }
        
        session.invalidateAndCancel()
        self.isDownloading = false
        self.prepareDownloadingView()
    }
    
    func URLSession(session: NSURLSession,
               downloadTask: NSURLSessionDownloadTask,
                didWriteData bytesWritten: Int64,
                                 totalBytesWritten: Int64,
                                 totalBytesExpectedToWrite: Int64) {
        let writtenMB = totalBytesWritten         / (1024 * 1024) // 1MB
        let totalMB   = totalBytesExpectedToWrite / (1024 * 1024) // 1MB
        
        progressBar.setProgress(Float(writtenMB)/Float(totalMB), animated: true)
        progressLabel.text = String(format:"%d/%d MB", writtenMB, totalMB)
    }

}
