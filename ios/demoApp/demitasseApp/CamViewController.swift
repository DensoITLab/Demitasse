//
//  CamViewController.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/04/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import UIKit
import AVFoundation

class CamViewController: UIViewController, UITableViewDataSource, UITableViewDelegate {

    @IBOutlet weak var tableView: UITableView!
    
    var resultLabel     : UITextView!
    var focusFrame      : UIView!

    var mySession       : AVCaptureSession?             = nil
    var myDevice        : AVCaptureDevice?              = nil
    var myImageOutput   : AVCaptureStillImageOutput?    = nil
    
    var speechSynthesizer : AVSpeechSynthesizer!
    
    weak var model_     : Model?    = nil
    var wordsList_      : NSArray?  = nil
    var meanImageBlob_  : Blob?     = nil
    
    var resultTitle     : [String]? = nil
    var resultScore     : [Float]?  = nil
    var isVisibleResult : Bool      = false
    var resultTimer     : NSTimer?  = nil

    deinit {
        
        speechSynthesizer.delegate = nil
        
        if let aSession = mySession {
            
            aSession.stopRunning()
            
            for output in aSession.outputs {
                aSession.removeOutput(output as? AVCaptureOutput)
            }
            
            for input in aSession.inputs {
                aSession.removeInput(input as? AVCaptureInput)
            }
            mySession = nil
            myDevice  = nil
        }
    }
    
    func initLiveCamView() {
        
        mySession = AVCaptureSession()
        
        let devices = AVCaptureDevice.devices()
        for device in devices{
            if(device.position == AVCaptureDevicePosition.Back){
                myDevice = device as? AVCaptureDevice
                break
            }
        }

        do {
            let videoInput = try AVCaptureDeviceInput(device:myDevice)
            mySession!.addInput(videoInput)
        } catch  {
        
        }
        
        myImageOutput = AVCaptureStillImageOutput()
        
        let outputSettings : Dictionary<NSObject, AnyObject> = [kCVPixelBufferPixelFormatTypeKey : NSNumber(unsignedInt: kCVPixelFormatType_32BGRA)]
        myImageOutput!.outputSettings = outputSettings
        
        mySession!.addOutput(myImageOutput)
        
        let myVideoLayer : AVCaptureVideoPreviewLayer = AVCaptureVideoPreviewLayer(session:mySession)
        myVideoLayer.frame = self.view.bounds
        myVideoLayer.videoGravity = AVLayerVideoGravityResizeAspectFill
        
        self.view.layer.addSublayer(myVideoLayer)
        
        mySession!.startRunning()
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let nib = UINib(nibName:"ResultViewCell", bundle:nil)
        tableView.registerNib(nib, forCellReuseIdentifier:"ResultCell")

        resultTitle = [String]()
        resultScore = [Float]()
        
        // initialize other data
        if wordsList_ == nil {
            if let path : String = NSBundle.mainBundle().pathForResource("synset_words", ofType: "plist") {
                wordsList_ = NSArray(contentsOfFile:path)
            }
        }

        if meanImageBlob_ == nil {
            if let path : String = NSBundle.mainBundle().pathForResource("ilsvrc12_mean", ofType: "blob") {
                meanImageBlob_ = demitasseUtils.load_mean_image(path, normalize:false)
            }
        }
        
        // init live camera view
        self.initLiveCamView()
        
        // tool bar
        let toolbar = UIToolbar(frame:CGRectMake(0, self.view.bounds.size.height - 44, self.view.bounds.size.width, 44))
        toolbar.barStyle    = .Black
        toolbar.translucent = true
        
        let cameraButton    = UIBarButtonItem(barButtonSystemItem:.Camera,
                                            target:self,
                                            action:#selector(CamViewController.onClickButton(_:)))

        let settingsButton  = UIBarButtonItem(title: "\u{2699}\u{0000FE0E}",
                                              style: .Plain,
                                              target: self,
                                              action: #selector(CamViewController.onSettingButton(_:)))
        if let font = UIFont(name: "Helvetica", size: 24.0) {
            settingsButton.setTitleTextAttributes([NSFontAttributeName: font], forState: UIControlState.Normal)
        }
        
        
        let flexibleItem    = UIBarButtonItem(barButtonSystemItem:.FlexibleSpace, target:nil, action:nil)

        toolbar.tintColor = UIColor.whiteColor()
        toolbar.items = [ flexibleItem, cameraButton, flexibleItem, settingsButton ]
        self.view.addSubview(toolbar)
    
        // focus frame
        let frameOffset = (self.view.bounds.size.height - self.view.bounds.size.width) / 2
        focusFrame = UIView(frame: CGRectMake(16, frameOffset,
                                              self.view.bounds.size.width - 32,
                                              self.view.bounds.size.width - 32))
    
        focusFrame.layer.borderColor = UIColor.blueColor().CGColor
        focusFrame.layer.borderWidth = 1.0
        
        self.view.addSubview(focusFrame)

        // pull table view
        self.view.bringSubviewToFront(self.tableView)

        speechSynthesizer = AVSpeechSynthesizer()
        
        self.showResult(false, animated:false)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    override func shouldAutorotate() -> Bool {
        return false
    }
    
    func rotationNeededForImageCapturedWithDeviceOrientation(deviceOrientation:UIDeviceOrientation) -> UIImageOrientation {
        var rotationOrientation = UIImageOrientation.Right
        
        switch (deviceOrientation) {
        case UIDeviceOrientation.PortraitUpsideDown:
            rotationOrientation = UIImageOrientation.Left
            
        case UIDeviceOrientation.LandscapeRight:
            rotationOrientation = UIImageOrientation.Down
            
        case UIDeviceOrientation.LandscapeLeft:
            rotationOrientation = UIImageOrientation.Up
            
        case UIDeviceOrientation.Portrait:
            rotationOrientation = UIImageOrientation.Right

        default:
            rotationOrientation = UIImageOrientation.Right
        }
        
        return rotationOrientation;
    }
    
    func onResultTimer() {
        self.showResult(false, animated:true)
    }
    
    func showResult(state: Bool, animated:Bool) {

        let completion = {
            if state {
                self.tableView.reloadData()
                
                // reset timer
                if let aTimer = self.resultTimer {
                    aTimer.invalidate()
                    self.resultTimer = nil
                }

                // set timer
                self.resultTimer = NSTimer.scheduledTimerWithTimeInterval(10.0,
                                                                   target: self,
                                                                 selector: #selector(CamViewController.onResultTimer),
                                                                 userInfo: nil,
                                                                  repeats: false)
            }
        }
        
        guard self.isVisibleResult != state else {
            completion() // reset timer
            return
        }

        if animated {
            UIView.animateWithDuration(0.3,
            animations: {
                self.tableView.alpha = (state ? 1.0 : 0.0)
            },
            completion: { (result: Bool) in
                completion()
                self.isVisibleResult = state
            })
        } else {
            self.tableView.alpha = (state ? 1.0 : 0.0)
            completion()
            self.isVisibleResult = state
        }
    }
    

    func clearResult() {
        self.resultTitle!.removeAll()
        self.resultScore!.removeAll()
        self.tableView.reloadData()
    }
    
    func setResult(result: Blob) {
        var n   = 0
        if let topN  = result.getTopNIndex(6) {
            for obj in topN {
                if let item = obj as? NSNumber {
                    let idx     = item.intValue
                    let predict = result.getValue(idx)
                    let word    = self.wordsList_?.objectAtIndex(Int(idx)) as! NSDictionary
                    var names   = word["name"] as! NSString
                    
                    // remove double quote
                    if names.hasPrefix("\"") {
                        names = names.substringFromIndex(1)
                    }
                    if names.hasSuffix("\"") {
                        names = names.substringToIndex(names.length - 1)
                    }
                    
                    if n < 1 {
                        let namesArray = names.componentsSeparatedByString(",")
                        if namesArray.count > 0 {
                            var name = namesArray[0]
                            
                            if predict < 0.3 {
                                name += " ?"
                            }
                            
                            let speechUtterance   = AVSpeechUtterance(string: name as String)
                            speechUtterance.voice = AVSpeechSynthesisVoice(language:"en-US")
                            
                            self.speechSynthesizer.speakUtterance(speechUtterance)
                        }
                    }
                    
                    self.resultTitle!.append(names as String)
                    self.resultScore!.append(predict)
                }
                
                n += 1;
            }
        }
    }
    

    func onClickButton(sender: UIButton) {
        
        // clear result
        self.clearResult()
        
        let deviceOrientation = UIDevice.currentDevice().orientation
        let imageOrientation  = self.rotationNeededForImageCapturedWithDeviceOrientation(deviceOrientation)
        
        let myVideoConnection = myImageOutput!.connectionWithMediaType(AVMediaTypeVideo)
        
        self.myImageOutput!.captureStillImageAsynchronouslyFromConnection(myVideoConnection, completionHandler: { (imageDataBuffer, error) -> Void in

            let pixelBuffer = CMSampleBufferGetImageBuffer(imageDataBuffer)
            let ciimage     = CIImage(CVPixelBuffer:pixelBuffer!)
 /*
            // for testing, use sample image file
            let image     = UIImage(named:"sample01.png")!
            let ciimage   = CIImage(CGImage:image.CGImage!)
  */
            let newSize   = CGSizeMake(224, 224)
            
            // convert CIImage to Blob
            // capture still image always landscape left orientation
            let blob = Blob.createBlobFromCIImage(ciimage, size: newSize, orientation: imageOrientation, normalize:false)
            
            blob.subBlob(self.meanImageBlob_!)
            
            // predict image
            if let result = self.model_?.feedForward(blob) {
                self.setResult(result)
                self.showResult(true, animated:false)
            }
        })
    }
    
    @IBAction func onSettingButton(sender: UIButton) {
        if let mainVC = self.parentViewController as? RootViewController {
            mainVC.showSettingsView(true)
        }
    }
    
    // MARK: UITableViewDataSource
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        guard let titles = resultTitle else {
            return 0
        }

        return titles.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("ResultCell", forIndexPath:indexPath) as! ResultViewCell
        
        cell.titleLabel!.text  = resultTitle![indexPath.row]
        cell.scoreLabel!.text  = String(format:"%.4f", resultScore![indexPath.row])
        cell.scoreBar.progress = resultScore![indexPath.row]
        
        return cell
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        //
        
    }
}


