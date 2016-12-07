/*
Copyright (C) 2016 Denso IT Laboratory, Inc.
All Rights Reserved

Denso IT Laboratory, Inc. retains sole and exclusive ownership of all
intellectual property rights including copyrights and patents related to this
Software.

Permission is hereby granted, free of charge, to any person obtaining a copy
of the Software and accompanying documentation to use, copy, modify, merge,
publish, or distribute the Software or software derived from it for
non-commercial purposes, such as academic study, education and personal use,
subject to the following conditions:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

import UIKit


class SettingsViewController: UIViewController, UITableViewDataSource, UITableViewDelegate {

    @IBOutlet weak var tableView        : UITableView!
    @IBOutlet weak var closeButton      : UIButton!
    @IBOutlet weak var noticeLabel      : UILabel!

    var selectedIndex : Int  = 0


    override func viewDidLoad() {
        super.viewDidLoad()

        NSNotificationCenter.defaultCenter().addObserver(self, selector: #selector(SettingsViewController.onUpdateModelData), name: "updateModelData", object: nil)


        selectedIndex = ModelData.selectedModelDataIndex

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

    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        // refresh table view
        self.tableView.reloadData()
    }

    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)

    }

    func onUpdateModelData() {
        self.tableView.reloadData()
    }

    @IBAction func onCloseButton(sender: UIButton) {
        if let rootVC = self.parentViewController as? RootViewController {
            rootVC.closeChildViewController(true, completion: {
                rootVC.loadingModelData(self.selectedIndex, animated: true)
            })
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
            if indexPath.row == selectedIndex {
                cell.accessoryType = .Checkmark
            } else {
                cell.accessoryType = .None
            }
        } else {
            state = NSLocalizedString("Not Downloaded", comment:"")
            cell.accessoryType     = .DisclosureIndicator
        }

        if ModelData.isExecModelDataSize(indexPath.row) {
            cell.textLabel!.textColor = UIColor(red: 63/255.0, green: 28/255.0, blue: 0/255.0, alpha: 1.0)
        } else {
            cell.textLabel!.textColor = UIColor.lightGrayColor()
            state = NSLocalizedString("Not Supported", comment:"")
            cell.accessoryType = .None
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
            selectedIndex = indexPath.row
            tableView.reloadRowsAtIndexPaths(tableView.indexPathsForVisibleRows!, withRowAnimation: UITableViewRowAnimation.Fade)
        } else {
            if let mainVC = self.parentViewController as? RootViewController {
                mainVC.showDownloadView(true, selectedIndex:indexPath.row)
            }
        }
    }
}
