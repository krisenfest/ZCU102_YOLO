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
	// tpg with box size here
	Text {
		id: filterPresetLbl
		anchors.top: parent.top
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Filter Presets:")
		font.family: fontFamily.name
		font.bold: true
	}
	DropDownBtn{
		id: filterPresetBox
		anchors.top: parent.top
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterPresetLbl.right
		anchors.leftMargin: 20 * resoluteFrac
		selectedText: presetPatternSelected == -1 ? "Bypass" : (presetList.length ? presetList[presetPatternSelected] : "" )
		dropDnBtn.onClicked: {
			presetComboSelected = !presetComboSelected
		}
	}
	Text {
		id: filterCoeffTitle
		anchors.top: filterPresetLbl.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: parent.left
		anchors.leftMargin: 50 * resoluteFrac
		font.pixelSize: 13 * resoluteFrac
		text: qsTr("Coefficients:")
		font.family: fontFamily.name
		font.bold: true
//					visible: presetPatternSelected == presetList.length - 1
	}
	// spinners
	SpinnerC{
		id: filterCombo00
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterPresetBox.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterPresetLbl.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter00Coeff]
		minusButton.onClicked: {
			if(filter00Coeff != 0){
				filter00Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter00Coeff != filterCoeffList.length-1){
				filter00Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo01
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterPresetBox.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo00.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter01Coeff]
		minusButton.onClicked: {
			if(filter01Coeff != 0){
				filter01Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter01Coeff != filterCoeffList.length-1){
				filter01Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo02
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterPresetBox.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo01.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter02Coeff]
		minusButton.onClicked: {
			if(filter02Coeff != 0){
				filter02Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter02Coeff != filterCoeffList.length-1){
				filter02Coeff++;
				applyCoeff()
			}
		}
	}
	// Row 1

	SpinnerC{
		id: filterCombo10
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo00.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo00.left
		anchors.leftMargin: 0 * resoluteFrac
		filterValue: filterCoeffList[filter10Coeff]
		minusButton.onClicked: {
			if(filter10Coeff != 0){
				filter10Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter10Coeff != filterCoeffList.length-1){
				filter10Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo11
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo01.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo10.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter11Coeff]
		minusButton.onClicked: {
			if(filter11Coeff != 0){
				filter11Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter11Coeff != filterCoeffList.length-1){
				filter11Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo12
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo02.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo11.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter12Coeff]
		minusButton.onClicked: {
			if(filter12Coeff != 0){
				filter12Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter12Coeff != filterCoeffList.length-1){
				filter12Coeff++;
				applyCoeff()
			}
		}
	}
	// Row 2

	SpinnerC{
		id: filterCombo20
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo10.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo10.left
		anchors.leftMargin: 0 * resoluteFrac
		filterValue: filterCoeffList[filter20Coeff]
		minusButton.onClicked: {
			if(filter20Coeff != 0){
				filter20Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter20Coeff != filterCoeffList.length-1){
				filter20Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo21
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo11.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo20.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter21Coeff]
		minusButton.onClicked: {
			if(filter21Coeff != 0){
				filter21Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter21Coeff != filterCoeffList.length-1){
				filter21Coeff++;
				applyCoeff()
			}
		}
	}
	SpinnerC{
		id: filterCombo22
		showButtons: presetPatternSelected == presetList.length - 1
		anchors.top: filterCombo12.bottom
		anchors.topMargin: 20 * resoluteFrac
		anchors.left: filterCombo21.right
		anchors.leftMargin: 20 * resoluteFrac
		filterValue: filterCoeffList[filter22Coeff]
		minusButton.onClicked: {
			if(filter22Coeff != 0){
				filter22Coeff--;
				applyCoeff()
			}
		}
		plusButton.onClicked: {
			if(filter22Coeff != filterCoeffList.length-1){
				filter22Coeff++;
				applyCoeff()
			}
		}
	}
	/*A transparent layer on clicked will dismiss the open pop over combobox optinos*/
	TansparantLayer{
		visible: presetComboSelected
		tlayer.onClicked: {
			presetComboSelected = false
		}
	}
	// scroll views
	DropDownScrollVu{
		id:filterPresetScrl
		width: filterPresetBox.width
		anchors.top: filterPresetBox.bottom
		anchors.left: filterPresetBox.left
		visible: presetComboSelected
		listModel.model: presetList
		selecteItem: presetPatternSelected
		delgate: this
		function clicked(indexval){
			presetPatternSelected = indexval
			setpreset(indexval);
			presetComboSelected = false;
		}
	}
}
