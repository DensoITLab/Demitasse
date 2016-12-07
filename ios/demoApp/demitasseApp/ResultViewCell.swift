//
//  ResultViewCell.swift
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/06/29.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

import UIKit

class ResultViewCell: UITableViewCell {

    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var scoreLabel: UILabel!
    @IBOutlet weak var scoreBar:   UIProgressView!
    
    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }

    override func setSelected(selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }
    
}
