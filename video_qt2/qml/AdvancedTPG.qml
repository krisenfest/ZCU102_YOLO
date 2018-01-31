/******************************************************************************
 * (c) Copyright 2012-2016 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *******************************************************************************/

/*
 * This file defines video QT application TPG panel setting option custom component.
 */

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0

Rectangle{
	anchors.fill: parent
	color: "transparent"
	Text {
		id: tpgPatternTitle
		anchors.top: parent.top
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("TPG Pattern:")
		font.family: fontFamily.name
		font.bold: true
	}
	DropDownBtn{
		id: tpgBox
		anchors.top: parent.top
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		selectedText: tpgPatterns[tpgPatternSelected]
		dropDnBtn.onClicked: {
			tpgComboSelected = !tpgComboSelected
		}
	}

	Text {
		id: zoneHPlateTitle
		anchors.top: tpgPatternTitle.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Horizontal Zone Plate:")
		font.family: fontFamily.name
		font.bold: true
		visible: tpgPatternSelected == 9
	}

	SliderC{
		anchors.top: tpgBox.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		visible: tpgPatternSelected == 9
		sliderValue: zoneHPlateValue
		maxValue: maxZoneHPlateValue
		delegate: this
		function slideValChanged(val){
			zoneHPlateValue = val
			setZoneH(val)
		}
	}


	Text {
		id: zoneHPlateDeltaTitle
		anchors.top: zoneHPlateTitle.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Horizontal Zone Delta:")
		font.family: fontFamily.name
		font.bold: true
		visible: tpgPatternSelected == 9
	}

	SliderC{
		anchors.top: zoneHPlateTitle.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		visible: tpgPatternSelected == 9
		sliderValue: zoneHPlateDeltaValue
		maxValue: maxZoneHPlateDeltaValue
		delegate: this
		function slideValChanged(val){
			zoneHPlateDeltaValue = val
			setZoneHDelta(val)
		}
	}

	Text {
		id: zoneVPlateTitle
		anchors.top: zoneHPlateDeltaTitle.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Vertical Zone Plate:")
		font.family: fontFamily.name
		font.bold: true
		visible: tpgPatternSelected == 9
	}

	SliderC{
		anchors.top: zoneHPlateDeltaTitle.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		visible: tpgPatternSelected == 9
		sliderValue: zoneVPlateValue
		maxValue: maxZoneVPlateValue
		delegate: this
		function slideValChanged(val){
			zoneVPlateValue = val
			setZoneV(val)
		}
	}


	Text {
		id: zoneVPlateDeltaTitle
		anchors.top: zoneVPlateTitle.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Vertical Zone Delta:")
		font.family: fontFamily.name
		font.bold: true
		visible: tpgPatternSelected == 9
	}
	SliderC{
		anchors.top: zoneVPlateTitle.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		visible: tpgPatternSelected == 9
		sliderValue: zoneVPlateDeltaValue
		maxValue: maxZoneVPlateDeltaValue
		delegate: this
		function slideValChanged(val){
			zoneVPlateDeltaValue = val
			setZoneVDelta(val)
		}
	}

	Text {
		id: boxSpeedTxt
		anchors.top: zoneVPlateDeltaTitle.visible  ? zoneVPlateDeltaTitle.bottom : tpgBox.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Motion Speed:")
		font.family: fontFamily.name
		font.bold: true
		visible: true

	}
	SliderC{
		id:sliderBoxSpeed
		anchors.left: tpgBox.left
		anchors.leftMargin: 0 * resoluteFrac
		anchors.top: zoneVPlateDeltaTitle.visible  ? zoneVPlateDeltaTitle.bottom : tpgBox.bottom
		anchors.topMargin: 15 * resoluteFrac
		sliderValue: boxSpeedValue
		maxValue: maxBoxSpeedValue
		delegate: this
		function slideValChanged(val){
			boxSpeedValue = val
			setBoxSpeed(val)

		}
	}
	Text {
		id: foreGroundTitle
		anchors.top: sliderBoxSpeed.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Foreground Overlay:")
		font.family: fontFamily.name
		font.bold: true
	}
	DropDownBtn{
		id: fgBox
		anchors.top: sliderBoxSpeed.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: tpgPatternTitle.right
		anchors.leftMargin: 60 * resoluteFrac
		selectedText: foreGroundList[foreGroundSelectOpt]
		dropDnBtn.onClicked: {
			fgComboSelected = !fgComboSelected
		}
	}
	Text {
		id: boxColorText
		anchors.top: fgBox.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Box Color:")
		font.family: fontFamily.name
		font.bold: true
		visible: foreGroundSelectOpt == 1

	}
	Rectangle{
		id: boxColorCombo
		width: 250 * resoluteFrac
		height: 20 * resoluteFrac
		anchors.top: fgBox.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left:  tpgPatternTitle.right
		visible: foreGroundSelectOpt == 1
		anchors.leftMargin: 60 * resoluteFrac
		color:"#00000000"
		GridLayout{
			columns: 8
			rowSpacing: 0
			columnSpacing: 0
			Repeater{
				id: boxColorRepeater
				model: boxColorList
				Rectangle{
					Text {
						font.family: fontFamily.name
						font.pointSize: 13 * resoluteFrac
						anchors.horizontalCenter: parent.horizontalCenter
					}
					height: 20*resoluteFrac
					width: 20*resoluteFrac
					color: modelData
					border.color: boxColorbSelect == index ? "#0C95CD" : barColors
					border.width: 3 * resoluteFrac //boxColorbSelect == index ? 3 * resoluteFrac : 0
					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
						onClicked:{
							boxColorbSelect = index
							setBoxColor(boxYUVColors[boxColorbSelect])
							boxColorSelected= false;
						}
					}
				}
			}
		}
	}

	Text {
		id: boxSize
		anchors.top: boxColorCombo.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Box Size:")
		font.family: fontFamily.name
		font.bold: true
		visible: foreGroundSelectOpt == 1

	}
	SliderC{
		id:sliderBoxSize
		anchors.top: boxColorCombo.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left:  tpgPatternTitle.right
		visible: foreGroundSelectOpt == 1
		anchors.leftMargin: 60 * resoluteFrac
		sliderValue: boxSizeValue
		maxValue: maxBoxSizeValue
		delegate: this
		function slideValChanged(val){
			boxSizeValue = val
			setBoxSize(val)
		}
	}

	// Cross hairs options

	Text {
		id: crossRowsText
		anchors.top: foreGroundTitle.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Cross Hairs Row:")
		font.family: fontFamily.name
		font.bold: true
		visible: foreGroundSelectOpt == 2

	}

	SliderC{
		id:crossRows
		anchors.left: fgBox.left
		anchors.top: fgBox.bottom
		anchors.leftMargin: 0 * resoluteFrac
		anchors.topMargin: 15 * resoluteFrac
		visible: foreGroundSelectOpt == 2
		sliderValue: crossRowsValue
		maxValue: maxCrossRowsValue
		delegate: this
		function slideValChanged(val){
			crossRowsValue = val
			setCrossRows(val)
		}
	}


	Text {
		id: crossColumnsText
		anchors.top: crossRowsText.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Cross Hairs Column:")
		font.family: fontFamily.name
		font.bold: true
		visible: foreGroundSelectOpt == 2

	}
	SliderC{
		anchors.top: crossRows.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left:  crossRows.left
		visible: foreGroundSelectOpt == 2
		anchors.leftMargin: 0 * resoluteFrac
		sliderValue: crossColumnsValue
		maxValue: maxCrossColumnsValue
		delegate: this
		function slideValChanged(val){
			crossColumnsValue = val
			setCrossColumns(val)
		}
	}

	/*A transparent layer on clicked will dismiss the open pop over combobox optinos*/
	TansparantLayer{
		visible: tpgComboSelected || fgComboSelected || boxColorSelected
		tlayer.onClicked: {
			tpgComboSelected = false
			fgComboSelected = false
			boxColorSelected = false
		}
	}

	// dropdown area for TPG
	DropDownScrollVu{
		id:tpgScrl
		width: tpgBox.width
		anchors.top: tpgBox.bottom
		anchors.left: tpgBox.left
		visible: tpgComboSelected
		listModel.model: tpgPatterns
		selecteItem: tpgPatternSelected
		delgate: this
		function clicked(indexval){
			tpgPatternSelected = indexval
			setTPGPattern(indexval);
			tpgComboSelected = false;
		}
	}

	DropDownScrollVu{
		id:fgBxScrl
		width: fgBox.width
		anchors.top: fgBox.bottom
		anchors.left: fgBox.left
		visible: fgComboSelected
		listModel.model: foreGroundList
		selecteItem: foreGroundSelectOpt
		delgate: this
		function clicked(indexval){
			foreGroundSelectOpt = indexval
			setFgOverlay(indexval)
			fgComboSelected = false;
		}
	}
}
