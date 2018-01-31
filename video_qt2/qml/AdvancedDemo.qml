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
 * This file defines video QT application Advanced filter settings custom component.
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0

Rectangle{
	anchors.fill: parent
	color: "transparent"
	property var seconds: 5
	property var maxDemoSeqCount: 9
	property var demoSeqCount: 1
	property int ddWidth: 120
	property var sourcesList: getsourcesList()
	property var filterTypeList: getFilterTypesList()
	property var filterModesList: filterModes

	property var transpLayerindex: 0

	// Time interval spinner
	Text {
		id: secondsTitle
		anchors.top: parent.top
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 25 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Time Interval:")
		font.family: fontFamily.name
		font.bold: true
	}
	// spinners
	SpinnerC{
		id: secondsSpinner
		showButtons: true
		anchors.top: parent.top
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: secondsTitle.right
		anchors.leftMargin: 80 * resoluteFrac
		filterValue: seconds
		minusButton.onClicked: {
			if(seconds != 1){
				seconds--;
				dataSource.updateDemoTimer(seconds)
			}
		}
		plusButton.onClicked: {
			if(seconds != 9){
				seconds++;
				dataSource.updateDemoTimer(seconds)
			}
		}
	}
	// Demo sequence spinner
	Text {
		id: demoListCountlbl
		anchors.top: secondsTitle.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 25 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Demo Sequence Count:")
		font.family: fontFamily.name
		font.bold: true
	}
	SpinnerC{
		id: demoListCountSpinner
		showButtons: true
		anchors.top: secondsTitle.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: secondsSpinner.left
		anchors.leftMargin: 0 * resoluteFrac
		filterValue: demoSeqCount
		minusButton.onClicked: {
			if(demoSeqCount != 1){
				demoSeqCount--;
				updateDemoSequence()
			}
		}
		plusButton.onClicked: {
			if(demoSeqCount != maxDemoSeqCount){
				demoSeqCount++;
				resetNewCounters(demoSeqCount)
				updateDemoSequence()
			}
		}
	}
	// Drop down Buttons
	// Sources
	Text {
		id: vSrcTitle
		anchors.top: demoListCountSpinner.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Video Source")
		font.family: fontFamily.name
		font.bold: true
	}
	// index labels
	Text{
		id: demoSeqLbl1
		text: "1"
		font.pixelSize: 13 * resoluteFrac
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 10 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl2
		text: "2"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 2
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 40 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl3
		text: "3"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 3
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 70 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl4
		text: "4"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 4
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 100 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl5
		text: "5"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 5
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 130 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl6
		text: "6"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 6
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 160 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl7
		text: "7"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 7
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 190 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl8
		text: "8"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 8
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 220 * resoluteFrac;}
	}
	Text{
		id: demoSeqLbl9
		text: "9"
		font.pixelSize: 13 * resoluteFrac
		visible: demoSeqCount >= 9
		anchors{left: parent.left; leftMargin: 25 * resoluteFrac; top: vSrcTitle.bottom; topMargin: 250 * resoluteFrac;}
	}

	// drop down buttons
	DropDownBtn{
		id: vSrcDropDown1
		width: ddWidth * resoluteFrac
		anchors.top: vSrcTitle.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[0].source] :"None"
		visible: demoSeqCount >= 1
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 11
		}
	}
	DropDownBtn{
		id: vSrcDropDown2
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown1.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[1].source] :"None"
		visible: demoSeqCount >= 2
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 21
		}
	}
	DropDownBtn{
		id: vSrcDropDown3
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown2.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[2].source] :"None"
		visible: demoSeqCount >= 3
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 31
		}
	}
	DropDownBtn{
		id: vSrcDropDown4
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown3.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[3].source] :"None"
		visible: demoSeqCount >= 4
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 41
		}
	}
	DropDownBtn{
		id: vSrcDropDown5
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown4.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[4].source] :"None"
		visible: demoSeqCount >= 5
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 51
		}
	}
	DropDownBtn{
		id: vSrcDropDown6
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown5.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[5].source] :"None"
		visible: demoSeqCount >= 6
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 61
		}
	}
	DropDownBtn{
		id: vSrcDropDown7
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown6.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[6].source] :"None"
		visible: demoSeqCount >= 7
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 71
		}
	}
	DropDownBtn{
		id: vSrcDropDown8
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown7.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[7].source] :"None"
		visible: demoSeqCount >= 8
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 81
		}
	}
	DropDownBtn{
		id: vSrcDropDown9
		width: ddWidth * resoluteFrac
		anchors.top: vSrcDropDown8.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		selectedText: sourcesList.length ? sourcesList[demoSequence[8].source] :"None"
		visible: demoSeqCount >= 9
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 91
		}
	}
	// Accelerator
	Text {
		id: acceleratorTitle
		anchors.top: demoListCountSpinner.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: vSrcDropDown1.right
		anchors.leftMargin: 20 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Accelerator")
		font.family: fontFamily.name
		font.bold: true
		visible: filterTypeList.length > 0
	}
	DropDownBtn{
		id: acceleratorDropDown1
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorTitle.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown1.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[0].filterType] : "None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 1)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 12
		}
	}
	DropDownBtn{
		id: acceleratorDropDown2
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown1.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown2.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[1].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 2)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 22
		}
	}
	DropDownBtn{
		id: acceleratorDropDown3
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown2.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown3.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[2].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 3)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 32
		}
	}
	DropDownBtn{
		id: acceleratorDropDown4
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown3.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown4.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[3].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 4)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 42
		}
	}
	DropDownBtn{
		id: acceleratorDropDown5
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown4.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown5.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[4].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 5)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 52
		}
	}
	DropDownBtn{
		id: acceleratorDropDown6
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown5.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown6.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[5].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 6)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 62
		}
	}
	DropDownBtn{
		id: acceleratorDropDown7
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown6.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown7.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[6].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 7)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 72
		}
	}
	DropDownBtn{
		id: acceleratorDropDown8
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown7.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown8.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[7].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 8)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 82
		}
	}
	DropDownBtn{
		id: acceleratorDropDown9
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorDropDown8.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: vSrcDropDown9.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterTypeList.length ? filterTypeList[demoSequence[8].filterType] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 9)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 92
		}
	}
	// Accelerator mode
	Text {
		id: acceleratorModeTitle
		anchors.top: demoListCountSpinner.bottom
		anchors.topMargin: 15 * resoluteFrac
		anchors.left: acceleratorDropDown1.right
		anchors.leftMargin: 20 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Accelerator Mode")
		font.family: fontFamily.name
		font.bold: true
		visible: filterTypeList.length > 0
	}
	DropDownBtn{
		id: acceleratorModeDropDown1
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeTitle.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown1.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[0].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 1)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 13
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown2
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown1.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown2.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[1].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 2)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 23
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown3
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown2.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown3.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[2].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 3)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 33
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown4
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown3.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown4.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[3].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 4)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 43
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown5
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown4.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown5.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[4].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 5)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 53
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown6
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown5.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown6.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[5].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 6)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 63
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown7
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown6.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown7.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[6].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 7)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 73
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown8
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown7.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown8.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[7].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 8)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 83
		}
	}
	DropDownBtn{
		id: acceleratorModeDropDown9
		width: ddWidth * resoluteFrac
		anchors.top: acceleratorModeDropDown8.bottom
		anchors.topMargin: 10 * resoluteFrac
		anchors.left: acceleratorDropDown9.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: filterModesList.length ? filterModesList[demoSequence[8].filterMode] :"None"
		visible: (filterTypeList.length > 0) && (demoSeqCount >= 9)
		dropDnBtn.onClicked: {
			transpLayerindex = transpLayerindex ? 0 : 93
		}
	}
	/*A transparent layer on clicked will dismiss the open pop over combobox optinos*/
	TansparantLayer{
		visible: transpLayerindex
		tlayer.onClicked: {
			transpLayerindex = 0
		}
		onVisibleChanged: {
			if(transpLayerindex == 0){
				updateDemoSequence()
			}
		}
	}
	// scroll views
	// Sources
	DropDownScrollVu{
		id:vSrcScrl1
		width: vSrcDropDown1.width
		anchors.top: vSrcDropDown1.bottom
		anchors.left: vSrcDropDown1.left
		visible: transpLayerindex == 11
		listModel.model: sourcesList
		selecteItem: demoSequence[0].source
		delgate: this
		function clicked(indexval){
			demoSequence[0].source = indexval
			vSrcScrl1.selecteItem = indexval
			vSrcDropDown1.selectedText = sourcesList.length ? sourcesList[demoSequence[0].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl2
		width: vSrcDropDown2.width
		anchors.top: vSrcDropDown2.bottom
		anchors.left: vSrcDropDown2.left
		visible: transpLayerindex == 21
		listModel.model: sourcesList
		selecteItem: demoSequence[1].source
		delgate: this
		function clicked(indexval){
			demoSequence[1].source = indexval
			vSrcScrl2.selecteItem = indexval
			vSrcDropDown2.selectedText = sourcesList.length ? sourcesList[demoSequence[1].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl3
		width: vSrcDropDown3.width
		anchors.top: vSrcDropDown3.bottom
		anchors.left: vSrcDropDown3.left
		visible: transpLayerindex == 31
		listModel.model: sourcesList
		selecteItem: demoSequence[2].source
		delgate: this
		function clicked(indexval){
			demoSequence[2].source = indexval
			vSrcScrl3.selecteItem = indexval
			vSrcDropDown3.selectedText = sourcesList.length ? sourcesList[demoSequence[2].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl4
		width: vSrcDropDown4.width
		anchors.top: vSrcDropDown4.bottom
		anchors.left: vSrcDropDown4.left
		visible: transpLayerindex == 41
		listModel.model: sourcesList
		selecteItem: demoSequence[3].source
		delgate: this
		function clicked(indexval){
			demoSequence[3].source = indexval
			vSrcScrl4.selecteItem = indexval
			vSrcDropDown4.selectedText = sourcesList.length ? sourcesList[demoSequence[3].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl5
		width: vSrcDropDown5.width
		anchors.top: vSrcDropDown5.bottom
		anchors.left: vSrcDropDown5.left
		visible: transpLayerindex == 51
		listModel.model: sourcesList
		selecteItem: demoSequence[4].source
		delgate: this
		function clicked(indexval){
			demoSequence[4].source = indexval
			vSrcScrl5.selecteItem = indexval
			vSrcDropDown5.selectedText = sourcesList.length ? sourcesList[demoSequence[4].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl6
		width: vSrcDropDown6.width
		anchors.top: vSrcDropDown6.bottom
		anchors.left: vSrcDropDown6.left
		visible: transpLayerindex == 61
		listModel.model: sourcesList
		selecteItem: demoSequence[5].source
		delgate: this
		function clicked(indexval){
			demoSequence[5].source = indexval
			vSrcScrl6.selecteItem = indexval
			vSrcDropDown6.selectedText = sourcesList.length ? sourcesList[demoSequence[5].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl7
		width: vSrcDropDown7.width
		anchors.top: vSrcDropDown7.bottom
		anchors.left: vSrcDropDown7.left
		visible: transpLayerindex == 71
		listModel.model: sourcesList
		selecteItem: demoSequence[6].source
		delgate: this
		function clicked(indexval){
			demoSequence[6].source = indexval
			vSrcScrl7.selecteItem = indexval
			vSrcDropDown7.selectedText = sourcesList.length ? sourcesList[demoSequence[6].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl8
		width: vSrcDropDown8.width
		anchors.top: vSrcDropDown8.bottom
		anchors.left: vSrcDropDown8.left
		visible: transpLayerindex == 81
		listModel.model: sourcesList
		selecteItem: demoSequence[7].source
		delgate: this
		function clicked(indexval){
			demoSequence[7].source = indexval
			vSrcScrl8.selecteItem = indexval
			vSrcDropDown8.selectedText = sourcesList.length ? sourcesList[demoSequence[7].source] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:vSrcScrl9
		width: vSrcDropDown9.width
		anchors.top: vSrcDropDown9.bottom
		anchors.left: vSrcDropDown9.left
		visible: transpLayerindex == 91
		listModel.model: sourcesList
		selecteItem: demoSequence[8].source
		delgate: this
		function clicked(indexval){
			demoSequence[8].source = indexval
			vSrcScrl9.selecteItem = indexval
			vSrcDropDown9.selectedText = sourcesList.length ? sourcesList[demoSequence[8].source] : "None"
			transpLayerindex = 0;
		}
	}
	// Accelerator
	DropDownScrollVu{
		id:fTypeScrl1
		width: acceleratorDropDown1.width
		anchors.top: acceleratorDropDown1.bottom
		anchors.left: acceleratorDropDown1.left
		visible: transpLayerindex == 12
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[0].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[0].filterType = indexval
			fTypeScrl1.selecteItem = indexval
			acceleratorDropDown1.selectedText = filterTypeList.length ? filterTypeList[demoSequence[0].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl2
		width: acceleratorDropDown2.width
		anchors.top: acceleratorDropDown2.bottom
		anchors.left: acceleratorDropDown2.left
		visible: transpLayerindex == 22
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[1].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[1].filterType = indexval
			fTypeScrl2.selecteItem = indexval
			acceleratorDropDown2.selectedText = filterTypeList.length ? filterTypeList[demoSequence[1].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl3
		width: acceleratorDropDown3.width
		anchors.top: acceleratorDropDown3.bottom
		anchors.left: acceleratorDropDown3.left
		visible: transpLayerindex == 32
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[2].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[2].filterType = indexval
			fTypeScrl3.selecteItem = indexval
			acceleratorDropDown3.selectedText = filterTypeList.length ? filterTypeList[demoSequence[2].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl4
		width: acceleratorDropDown4.width
		anchors.top: acceleratorDropDown4.bottom
		anchors.left: acceleratorDropDown4.left
		visible: transpLayerindex == 42
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[3].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[3].filterType = indexval
			fTypeScrl4.selecteItem = indexval
			acceleratorDropDown4.selectedText = filterTypeList.length ? filterTypeList[demoSequence[3].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl5
		width: acceleratorDropDown5.width
		anchors.top: acceleratorDropDown5.bottom
		anchors.left: acceleratorDropDown5.left
		visible: transpLayerindex == 52
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[4].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[4].filterType = indexval
			fTypeScrl5.selecteItem = indexval
			acceleratorDropDown5.selectedText = filterTypeList.length ? filterTypeList[demoSequence[4].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl6
		width: acceleratorDropDown6.width
		anchors.top: acceleratorDropDown6.bottom
		anchors.left: acceleratorDropDown6.left
		visible: transpLayerindex == 62
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[5].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[5].filterType = indexval
			fTypeScrl6.selecteItem = indexval
			acceleratorDropDown6.selectedText = filterTypeList.length ? filterTypeList[demoSequence[5].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl7
		width: acceleratorDropDown7.width
		anchors.top: acceleratorDropDown7.bottom
		anchors.left: acceleratorDropDown7.left
		visible: transpLayerindex == 72
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[6].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[6].filterType = indexval
			fTypeScrl7.selecteItem = indexval
			acceleratorDropDown7.selectedText = filterTypeList.length ? filterTypeList[demoSequence[6].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl8
		width: acceleratorDropDown8.width
		anchors.top: acceleratorDropDown8.bottom
		anchors.left: acceleratorDropDown8.left
		visible: transpLayerindex == 82
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[7].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[7].filterType = indexval
			fTypeScrl8.selecteItem = indexval
			acceleratorDropDown8.selectedText = filterTypeList.length ? filterTypeList[demoSequence[7].filterType] : -1
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fTypeScrl9
		width: acceleratorDropDown9.width
		anchors.top: acceleratorDropDown9.bottom
		anchors.left: acceleratorDropDown9.left
		visible: transpLayerindex == 92
		listModel.model: filterTypeList
		selecteItem: filterTypeList.length ? demoSequence[8].filterType : -1
		delgate: this
		function clicked(indexval){
			demoSequence[8].filterType = indexval
			fTypeScrl9.selecteItem = indexval
			acceleratorDropDown9.selectedText = filterTypeList.length ? filterTypeList[demoSequence[8].filterType] : -1
			transpLayerindex = 0;
		}
	}
	// Accelerator mode
	DropDownScrollVu{
		id:fModeScrl1
		width: acceleratorModeDropDown1.width
		anchors.top: acceleratorModeDropDown1.bottom
		anchors.left: acceleratorModeDropDown1.left
		visible: transpLayerindex == 13
		listModel.model: filterModesList
		selecteItem: demoSequence[0].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[0].filterMode = indexval
			fModeScrl1.selecteItem = indexval
			acceleratorModeDropDown1.selectedText = filterModesList.length ? filterModesList[demoSequence[0].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl2
		width: acceleratorModeDropDown2.width
		anchors.top: acceleratorModeDropDown2.bottom
		anchors.left: acceleratorModeDropDown2.left
		visible: transpLayerindex == 23
		listModel.model: filterModesList
		selecteItem: demoSequence[1].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[1].filterMode = indexval
			fModeScrl2.selecteItem = indexval
			acceleratorModeDropDown2.selectedText = filterModesList.length ? filterModesList[demoSequence[1].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl3
		width: acceleratorModeDropDown3.width
		anchors.top: acceleratorModeDropDown3.bottom
		anchors.left: acceleratorModeDropDown3.left
		visible: transpLayerindex == 33
		listModel.model: filterModesList
		selecteItem: demoSequence[2].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[2].filterMode = indexval
			fModeScrl3.selecteItem = indexval
			acceleratorModeDropDown3.selectedText = filterModesList.length ? filterModesList[demoSequence[2].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl4
		width: acceleratorModeDropDown4.width
		anchors.top: acceleratorModeDropDown4.bottom
		anchors.left: acceleratorModeDropDown4.left
		visible: transpLayerindex == 43
		listModel.model: filterModesList
		selecteItem: demoSequence[3].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[3].filterMode = indexval
			fModeScrl4.selecteItem = indexval
			acceleratorModeDropDown4.selectedText = filterModesList.length ? filterModesList[demoSequence[3].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl5
		width: acceleratorModeDropDown5.width
		anchors.top: acceleratorModeDropDown5.bottom
		anchors.left: acceleratorModeDropDown5.left
		visible: transpLayerindex == 53
		listModel.model: filterModesList
		selecteItem: demoSequence[4].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[4].filterMode = indexval
			fModeScrl5.selecteItem = indexval
			acceleratorModeDropDown5.selectedText = filterModesList.length ? filterModesList[demoSequence[4].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl6
		width: acceleratorModeDropDown6.width
		anchors.top: acceleratorModeDropDown6.bottom
		anchors.left: acceleratorModeDropDown6.left
		visible: transpLayerindex == 63
		listModel.model: filterModesList
		selecteItem: demoSequence[5].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[5].filterMode = indexval
			fModeScrl6.selecteItem = indexval
			acceleratorModeDropDown6.selectedText = filterModesList.length ? filterModesList[demoSequence[5].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl7
		width: acceleratorModeDropDown7.width
		anchors.top: acceleratorModeDropDown7.bottom
		anchors.left: acceleratorModeDropDown7.left
		visible: transpLayerindex == 73
		listModel.model: filterModesList
		selecteItem: demoSequence[6].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[6].filterMode = indexval
			fModeScrl7.selecteItem = indexval
			acceleratorModeDropDown7.selectedText = filterModesList.length ? filterModesList[demoSequence[6].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl8
		width: acceleratorModeDropDown8.width
		anchors.top: acceleratorModeDropDown8.bottom
		anchors.left: acceleratorModeDropDown8.left
		visible: transpLayerindex == 83
		listModel.model: filterModesList
		selecteItem: demoSequence[7].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[7].filterMode = indexval
			fModeScrl8.selecteItem = indexval
			acceleratorModeDropDown8.selectedText = filterModesList.length ? filterModesList[demoSequence[7].filterMode] : "None"
			transpLayerindex = 0;
		}
	}
	DropDownScrollVu{
		id:fModeScrl9
		width: acceleratorModeDropDown9.width
		anchors.top: acceleratorModeDropDown9.bottom
		anchors.left: acceleratorModeDropDown9.left
		visible: transpLayerindex == 93
		listModel.model: filterModesList
		selecteItem: demoSequence[8].filterMode
		delgate: this
		function clicked(indexval){
			demoSequence[8].filterMode = indexval
			fModeScrl9.selecteItem = indexval
			acceleratorModeDropDown9.selectedText = filterModesList.length ? filterModesList[demoSequence[8].filterMode] : "None"
			transpLayerindex = 0;
		}
	}

	// Functions
	function getsourcesList(){
		var sList = [];
		for (var i = 0; i < videoSourceList.length; i++){
			sList.push(videoSourceList[i].shortName)
		}
		return sList;
	}
	function getFilterTypesList(){
		var sList = [];
		for (var i = 0; i < filterTypeSourceList.length; i++){
			sList.push(filterTypeSourceList[i].shortName)
		}
		return sList;
	}
	function updateDemoSequence(){
		// Read all the values of demo mode, Create struct and send to maincontroller for processing
		var seq = []
		for(var i = 0; i < demoSeqCount; i++){
			var obj = {}
			obj["source"] = videoSourceList[demoSequence[i].source].sourceIndex;
			obj["filterType"] = filterTypeList.length ? filterTypeSourceList[demoSequence[i].filterType].sourceIndex : -1;
			obj["filterMode"] = demoSequence[i].filterMode;
			seq.push(obj);
		}
		dataSource.demoSequence(seq)
		dataSource.updateDemoTimer(seconds)
	}
	function resetNewCounters(index){
		switch(index){
		case 2:
			demoSequence[1].source = 0;
			vSrcScrl2.selecteItem = 0
			vSrcDropDown2.selectedText = sourcesList.length ? sourcesList[demoSequence[1].source] : "None"
			demoSequence[1].filterType = 0
			fTypeScrl2.selecteItem = 0
			acceleratorDropDown2.selectedText = filterTypeList.length ? filterTypeList[demoSequence[1].filterType] : "None"
			demoSequence[1].filterMode = 0
			fModeScrl2.selecteItem = 0
			acceleratorModeDropDown2.selectedText = filterModesList.length ? filterModesList[demoSequence[1].filterMode] : "None"
			break;
		case 3:
			demoSequence[2].source = 0;
			vSrcScrl3.selecteItem = 0
			vSrcDropDown3.selectedText = sourcesList.length ? sourcesList[demoSequence[2].source] : "None"
			demoSequence[2].filterType = 0
			fTypeScrl3.selecteItem = 0
			acceleratorDropDown3.selectedText = filterTypeList.length ? filterTypeList[demoSequence[2].filterType] : "None"
			demoSequence[2].filterMode = 0
			fModeScrl3.selecteItem = 0
			acceleratorModeDropDown3.selectedText = filterModesList.length ? filterModesList[demoSequence[2].filterMode] : "None"
			break;
		case 4:
			demoSequence[3].source = 0;
			vSrcScrl4.selecteItem = 0
			vSrcDropDown4.selectedText = sourcesList.length ? sourcesList[demoSequence[3].source] : "None"
			demoSequence[3].filterType = 0
			fTypeScrl4.selecteItem = 0
			acceleratorDropDown4.selectedText = filterTypeList.length ? filterTypeList[demoSequence[3].filterType] : "None"
			demoSequence[3].filterMode = 0
			fModeScrl4.selecteItem = 0
			acceleratorModeDropDown4.selectedText = filterModesList.length ? filterModesList[demoSequence[3].filterMode] : "None"
			break;
		case 5:
			demoSequence[4].source = 0;
			vSrcScrl5.selecteItem = 0
			vSrcDropDown5.selectedText = sourcesList.length ? sourcesList[demoSequence[4].source] : "None"
			demoSequence[4].filterType = 0
			fTypeScrl5.selecteItem = 0
			acceleratorDropDown5.selectedText = filterTypeList.length ? filterTypeList[demoSequence[4].filterType] : "None"
			demoSequence[4].filterMode = 0
			fModeScrl5.selecteItem = 0
			acceleratorModeDropDown5.selectedText = filterModesList.length ? filterModesList[demoSequence[4].filterMode] : "None"
			break;
		case 6:
			demoSequence[5].source = 0;
			vSrcScrl6.selecteItem = 0
			vSrcDropDown6.selectedText = sourcesList.length ? sourcesList[demoSequence[5].source] : "None"
			demoSequence[5].filterType = 0
			fTypeScrl6.selecteItem = 0
			acceleratorDropDown6.selectedText = filterTypeList.length ? filterTypeList[demoSequence[5].filterType] : "None"
			demoSequence[5].filterMode = 0
			fModeScrl6.selecteItem = 0
			acceleratorModeDropDown6.selectedText = filterModesList.length ? filterModesList[demoSequence[5].filterMode] : "None"
			break;
		case 7:
			demoSequence[6].source = 0;
			vSrcScrl7.selecteItem = 0
			vSrcDropDown7.selectedText = sourcesList.length ? sourcesList[demoSequence[6].source] : "None"
			demoSequence[6].filterType = 0
			fTypeScrl7.selecteItem = 0
			acceleratorDropDown7.selectedText = filterTypeList.length ? filterTypeList[demoSequence[6].filterType] : "None"
			demoSequence[6].filterMode = 0
			fModeScrl7.selecteItem = 0
			acceleratorModeDropDown7.selectedText = filterModesList.length ? filterModesList[demoSequence[6].filterMode] : "None"
			break;
		case 8:
			demoSequence[7].source = 0;
			vSrcScrl8.selecteItem = 0
			vSrcDropDown8.selectedText = sourcesList.length ? sourcesList[demoSequence[7].source] : "None"
			demoSequence[7].filterType = 0
			fTypeScrl8.selecteItem = 0
			acceleratorDropDown8.selectedText = filterTypeList.length ? filterTypeList[demoSequence[7].filterType] : "None"
			demoSequence[7].filterMode = 0
			fModeScrl8.selecteItem = 0
			acceleratorModeDropDown8.selectedText = filterModesList.length ? filterModesList[demoSequence[7].filterMode] : "None"
			break;
		case 9:
			demoSequence[8].source = 0;
			vSrcScrl9.selecteItem = 0
			vSrcDropDown9.selectedText = sourcesList.length ? sourcesList[demoSequence[8].source] : "None"
			demoSequence[8].filterType = 0
			fTypeScrl9.selecteItem = 0
			acceleratorDropDown9.selectedText = filterTypeList.length ? filterTypeList[demoSequence[8].filterType] : "None"
			demoSequence[8].filterMode = 0
			fModeScrl9.selecteItem = 0
			acceleratorModeDropDown9.selectedText = filterModesList.length ? filterModesList[demoSequence[8].filterMode] : "None"
			break;
		default:
			break;
		}
	}
}
