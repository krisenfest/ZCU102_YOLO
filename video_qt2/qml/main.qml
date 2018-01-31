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
 * This file defines video QT application user interface.
 */

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import QtCharts 2.1

import "qrc:///custom/qml"
Rectangle {
	visible: true
	id: root

	property int sobleToggle :0
	property bool play: false
	property int videoInput: 0
	property int filterTypeInput: 0
	property int previousVideoInput: -1
	property int filterinput: 0
	property int tpgPatternSelected: 13
	property bool statsDisplay: true
	property bool plotDisplay: true
	property bool invertEnabled: false
	property bool hideControlBar: false
	property bool auto_start: false
	property bool appRunning: true

	property var toolBarHeight: 58 * resoluteFrac
	property var errorMessageText: ""
	property var errorNameText: ""
	property var headerNames:["TPG Settings","2D Filter Settings"]
	property var filterModes: ["OFF","SW", "HW"]
	property int resolution: 720
	property var barColors: "#F0F7F7F7"
	property var barTitleColors: "#F0AAAAAA"
	property var cellColor: "#FFEEEEEE"
	property var cellHighlightColor: "#FFAAAAAA"
	property var borderColors: "#F0AAAAAA"
	property int boarderWidths: 1
	property var foreGroundList: ["None","Moving Box","Cross Hairs"]
	property int foreGroundSelectOpt: 1
	property int boxSelectSize: 3

	property var boxColorSelected: false
	property var boxColorList: ["white","yellow","cyan","green","magenta","red","blue","black"]
	property var boxYUVColors: [0xFF8080,0xE10094,0xB2AA00,0X952B15,0X69D4EA,0X4C55FF,0x1DFF6B,0x008080]
	property var boxColorLabelList: ["White","Yellow","Cyan","Green","Magenta","Red","Blue","Black"];
	property int boxColorbSelect:3
	property int boxSpeedSelOption: 1
	property int boxSpeedValue: 4
	property int maxBoxSpeedValue: 255
	property int boxSizeValue: 150
	property int maxBoxSizeValue: 4095
	property int crossRowsValue: 100
	property int maxCrossRowsValue: imageResolutionHeight - 1 //4095
	property int crossColumnsValue: 100
	property int maxCrossColumnsValue: imageResolutionWidth - 1 //4095
	property int zoneHPlateValue: 30
	property int maxZoneHPlateValue: 65536
	property int zoneVPlateValue: 1
	property int maxZoneVPlateValue: 65536
	property int zoneHPlateDeltaValue: 0
	property int maxZoneHPlateDeltaValue: 65536
	property int zoneVPlateDeltaValue: 0
	property int maxZoneVPlateDeltaValue: 65536

	property int crossRowsSelOption: 2
	property int crossColumnsSelOption: 2
	property int zoneHSelOption: 1
	property int zoneVSelOption: 1

	property var filterCoeffList: [-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9]
	property var filter00Coeff: 10
	property var filter01Coeff: 9
	property var filter02Coeff: 9
	property var filter10Coeff: 9
	property var filter11Coeff: 10
	property var filter12Coeff: 9
	property var filter20Coeff: 9
	property var filter21Coeff: 9
	property var filter22Coeff: 10

	property var widthCoeffsBox: 50
	property var tooltipWidth: 0


	FontLoader { id: fontFamily; source: "/font/font/luxisr.ttf" }

	property var popupVisible: 0
	property var popupTitle: "Settings"
	property var tpgComboSelected: false
	property var fgComboSelected: false

	property var presetComboSelected: false
	property int previousePatterSelVal: 0
	property int presetPatternSelected: 1  //-1 for none. Default is set to edge (index 1 starting with 0)

	property var filterComboSelected: false
	property var filterComboSelOption : 0

	property var showMemoryThroughput: false

	property alias numericinputlabelText: numericInputLabel.text
	property alias numericfilterlabelText: numericFilterLabel.text
	property alias numericFilterTypeLabelText: numericFilterTypeLabel.text

	property alias fpsValueText: framesValueLabel.text
//	property alias videoOptSel: videoInput
//	property alias filterOptSel: filterinput


	property alias cp1Lbl: cpu1Destails.text
	property alias cp2Lbl: cpu2Destails.text
	property alias cp3Lbl: cpu3Destails.text
	property alias cp4Lbl: cpu4Destails.text

	property alias truPutVideoSrcLbl: videoSrcLbl.text
	property alias truPutFilterLbl: filterLbl.text
	property alias truPutDisplayPortLbl: displayPortLbl.text

	property var demoSequence: [{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0},
					{"source":0, "filterType":0, "filterMode":0}]

	signal playvideo(int val)
	signal setFilter(int val)
	signal setFilterType(int val)
	signal setInputType(int val)
//	signal setInvert(int val)
	signal setTPGPattern(int val)
	signal setpreset (int val)
//	signal setSensitivity(int max, int min)
	signal demoMode(bool start)
	signal setFgOverlay(int overlay)
	signal setBoxSize(int size)
	signal setBoxColor(int color)
	signal setBoxSpeed(int speed)
	signal setCrossRows(int rows)
	signal setCrossColumns(int columns)
	signal setZoneH(int H)
	signal setZoneHDelta(int H)
	signal setZoneV(int v)
	signal setZoneVDelta(int v)
	signal filterCoeff(int c00,int c01,int c02,int c10,int c11,int c12,int c20 ,int c21,int c22)
	Timer {
		id:timerABCD
		repeat: true
		interval: 5000
		onTriggered: {
			if(hideControlBar && appRunning){
				popupVisible = 0
				showPlayarea(false)
				showDemoarea(false)
				showInputarea(false)
				showFilterarea(false)
				showFilterTypeArea(false)
				toolBarItem.visible = false
			}
		}
	}
	function resetTimer(val){
		if(appRunning){
			timerABCD.restart()
			toolBarItem.visible = true
		}
	}
        /*Tool tip functions*/

	function toolTipOnEnter(x, y, message){
		toolTipRect.toolTipOnEnter(x, y, message)
	}
	function toolTipOnExit(){
		toolTipRect.toolTipOnExit()

	}

	function updateInput(status) {
		if(status){
			previousVideoInput = videoInput
			if(play)
				sourceSettingsButtonImage.source = videoSourceList[videoInput].hasPanel ? "qrc:///imgs/images/gears@"+imageResolution+".png" : "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
		}
		if(previousVideoInput != -1){
			inputPopSettingsButton.enabled = videoSourceList[previousVideoInput].hasPanel
			videoInput = previousVideoInput
			videoSrcOptionsSV.resetSource(videoInput)
		}else if(!status){
			play = false
		}
		numericInputLabel.text = videoSourceList[videoInput].shortName
		return ""
	}
	color: play || auto_start? "#00000000" : "#FF000000"
	/********common pop up********/
	// Common Popup holder
	PopupHolder{
		id: popup
	}
	/*************************/
	// Numeric panel
	Rectangle{
		id: numericPanel
		width: 340 * resoluteFrac
		height: numColumnLayout.childrenRect.height
		border.color: borderColors
		border.width: boarderWidths
		color: barColors
		visible: statsDisplay
		anchors{
			left: parent.left
			top: parent.top
			leftMargin: 30 * resoluteFrac
			topMargin: 30 * resoluteFrac
		}
		ColumnLayout{
			id: numColumnLayout
			width: 340 * resoluteFrac
			spacing: 1
			Rectangle{
				width: parent.width
				height: 35 * resoluteFrac
				color: barTitleColors
				anchors.top: parent.top
				anchors.topMargin: 0
				RowLayout{
					anchors.fill: parent
					Label{
						text: "  Video Info"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.bold: true
						font.pointSize: 15 * resoluteFrac
						font.family: fontFamily.name
					}
				}

			}
			Rectangle{
				width: parent.width - 30
				height: 25 * resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Display Device:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						text: displayDeviceType
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}
			}
			Rectangle{
				width: parent.width - 30
				height: 25 * resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
//                radius: 5
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Display Resolution:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						text: resolution+"p"
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}

			}
			Rectangle{
				width: parent.width - 30
				height: 25 * resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
				radius: 5
				visible: (!auto_start)
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Frame Rate:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						id: framesValueLabel
						text: "0 fps"
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}
			}
			Rectangle{
				width: parent.width - 30
				height: 25* resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
//                radius: 5
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Video Source:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						id: numericInputLabel
						text: videoSourceList.length?videoSourceList[0].shortName:"No Video Sources Available"
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}

			}
			Rectangle{
				width: parent.width - 30
				height: 25* resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
				visible: filter2DEnabled
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Accelerator:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						id:numericFilterTypeLabel
						objectName: "numericFilterTypeLabel"
						text: filterTypeSourceList.length ? filterTypeSourceList[filterTypeInput].displayName : ""
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}
			}
			Rectangle{
				width: parent.width - 30
				height: 25* resoluteFrac
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
				visible: filter2DEnabled
//                radius: 5
				RowLayout{
					anchors.fill: parent
					Label{
						text: "Accelerator Mode:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}

					Label{
						id:numericFilterLabel
						objectName: "numericFilterLabel"
						text: filterModes[filterinput]
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}

			}
			Rectangle{
				width: parent.width - 30
				height: (filter2DEnabled  && (filterTypeSourceList.length ? (filterTypeSourceList[filterTypeInput].displayName === "2D Filter") : false)) ? 25* resoluteFrac : 0
				anchors.leftMargin: 15
				anchors.left: parent.left
				color: "transparent"
				visible: filter2DEnabled  && (filterTypeSourceList.length ? (filterTypeSourceList[filterTypeInput].displayName === "2D Filter") : false)
				RowLayout{
					anchors.fill: parent
					Label{
						text: "2D Filter Preset:"
						anchors.leftMargin: 10
						anchors.left: parent.left
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
					Label{
						text: presetList.length ? presetList[presetPatternSelected] : "NA"
						font.bold: true
						anchors.right: parent.right
						anchors.rightMargin: 10
						font.pointSize: 13 * resoluteFrac
						font.family: fontFamily.name
					}
				}

			}
		}
	}
	Flipable{
		id: flipable
		property int txtVuWidth: 240
		property int txtVuHeight: 140
                width: side == Flipable.Front ? 450 * resoluteFrac : flipable.txtVuWidth * resoluteFrac
                height: side == Flipable.Front ? 300 * resoluteFrac : flipable.txtVuHeight * resoluteFrac
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 30 * resoluteFrac
                anchors.topMargin: 30 * resoluteFrac
                property bool flipped: false
                back: Rectangle{
			visible: plotDisplay
			width: flipable.txtVuWidth * resoluteFrac; height: flipable.txtVuHeight * resoluteFrac; anchors.right: parent.right; color: barColors
			ColumnLayout{
				anchors.fill: parent
				spacing: 1
				Rectangle{
					anchors.top: parent.top
					anchors.topMargin: 0
					width: parent.width
					height: 35 * resoluteFrac
					color: barTitleColors
					RowLayout{
						anchors.fill: parent
						Label{
							text: "CPU Utilization"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.bold: true
							font.pointSize: 15 * resoluteFrac
							font.family: fontFamily.name
						}
					}

				}
				Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "CPU 1:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: cpu1Destails
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}

				}
				Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "CPU 2:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: cpu2Destails
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}

				}Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "CPU 3:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: cpu3Destails
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}
				}Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "CPU 4:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: cpu4Destails
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}
				}
			}
			border.color: borderColors
			border.width: boarderWidths
                }
                front:	Rectangle {
			id: cpuUtilizationPlot
			width: 450 * resoluteFrac
			height: 300 * resoluteFrac
			visible: plotDisplay
			color: barColors
			border.color: borderColors
			border.width: boarderWidths
			ChartView {
				id: chart_line_CPU
				anchors.fill: parent
				animationOptions: ChartView.NoAnimation
				property bool openGL: true
				backgroundRoundness: 0
				legend.alignment: Qt.AlignBottom
				legend.font.pixelSize: resoluteFrac == 1 ? 12 : (resoluteFrac == 2 ? 24 : 9 * resoluteFrac)
				titleFont.pixelSize: 13* resoluteFrac
				legend.visible: true
				margins.bottom: 0
				margins.top: 45 * resoluteFrac
				margins.left: 10 * resoluteFrac
				plotAreaColor: "transparent"
				backgroundColor: "transparent"
				Rectangle{
					width: parent.width
					height: 35 * resoluteFrac
					color: barTitleColors
					anchors.top: parent.top
					anchors.topMargin: 0
					anchors.left: parent.left
					anchors.leftMargin: 0
					Label{
						text: "CPU Utilization (%)"
						anchors.leftMargin: 20
						anchors.left: parent.left
						height: parent.height
						verticalAlignment: Text.AlignVCenter
						font.bold: true
						font.pointSize: 15 * resoluteFrac
						font.family: fontFamily.name
					}
				}
				ValueAxis {
					id: axisYcpu
					min: 0
					max: 100
					labelFormat: "%d"
					labelsFont.pointSize: 10 * resoluteFrac
					labelsColor: "black"
					gridLineColor: "black"
					lineVisible: false
                                }
                                ValueAxis {
					id: axisXcpu
					min: 0
					max: 60
					labelsVisible: false
					gridLineColor: "black"
					lineVisible: false
					labelsFont.pointSize: 1 * resoluteFrac
                                }
                                SplineSeries {
					name: "CPU 1"
					axisX: axisXcpu
					axisY: axisYcpu
					useOpenGL: chart_line_CPU.openGL
                                }
                                SplineSeries {
					name: "CPU 2"
					axisX: axisXcpu
					axisY: axisYcpu
					useOpenGL: chart_line_CPU.openGL
                                }
                                SplineSeries {
					name: "CPU 3"
					axisX: axisXcpu
					axisY: axisYcpu
					useOpenGL: chart_line_CPU.openGL
                                }
                                SplineSeries {
					name: "CPU 4"
					axisX: axisXcpu
					axisY: axisYcpu
					useOpenGL: chart_line_CPU.openGL
					color: "red"
                                }
			}
                }
                transform: Rotation {
			id: rotation
			origin.x: flipable.width/2
			origin.y: flipable.height/2
			axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
			angle: 0    // the default angle
                }

                states: State {
			name: "back"
			PropertyChanges { target: rotation; angle: 180 }
			when: flipable.flipped
                }

                transitions: Transition {
			NumberAnimation { target: rotation; property: "angle"; duration: 200 }
                }
                MouseArea {
			anchors.fill: parent
			hoverEnabled: true
			cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
			onClicked: flipable.flipped = !flipable.flipped
			onEntered: {
				toolTipOnEnter(parent.x + parent.width/3 + 10,parent.y  + parent.height/2 + 10, "Click to Toggle View")
			}
			onExited: {
				toolTipOnExit()
			}
                }
                Behavior on height{
			NumberAnimation {
				duration: 500
				easing.type: Easing.OutSine
			}
		}
	}

	Flipable{
		id: flipableMT
		property int txtVuWidthMT: 240
		property int txtVuHeightMT: 120
                width: side == Flipable.Front ? 450 * resoluteFrac : flipableMT.txtVuWidthMT * resoluteFrac
                height: side == Flipable.Front ? 300 * resoluteFrac : flipableMT.txtVuHeightMT * resoluteFrac
                anchors.right: parent.right
                anchors.rightMargin: 30 * resoluteFrac
                anchors.topMargin: 30 * resoluteFrac
                anchors.top: flipable.bottom
                property bool flipped: false
                visible: showMemoryThroughput && plotDisplay
                back: Rectangle{
			width: flipableMT.txtVuWidthMT * resoluteFrac; height: flipableMT.txtVuHeightMT * resoluteFrac; anchors.right: parent.right; color: barColors
			border.color: borderColors
			border.width: boarderWidths

			ColumnLayout{
				anchors.fill: parent
				spacing: 1
				Rectangle{
					anchors.top: parent.top
					anchors.topMargin: 0
					width: parent.width
					height: 35 * resoluteFrac
					color: barTitleColors
					RowLayout{
						anchors.fill: parent
						Label{
							text: "Memory Throughput"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.bold: true
							font.pointSize: 15 * resoluteFrac
							font.family: fontFamily.name
						}
					}
				}
				Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "Video Source:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: videoSrcLbl
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}
				}
				Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "Accelerator:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: filterLbl
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}

				}Rectangle{
					width: parent.width
					height: 25 * resoluteFrac
					color: "transparent"
					RowLayout{
						anchors.fill: parent
						Label{
							text: "Display:"
							anchors.leftMargin: 10
							anchors.left: parent.left
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
						Label{
							id: displayPortLbl
							font.bold: true
							anchors.right: parent.right
							anchors.rightMargin: 10
							font.pointSize: 13 * resoluteFrac
							font.family: fontFamily.name
						}
					}
				}
			}

                }
                front:	Rectangle {
			id: performanceRect
			width: 450 * resoluteFrac
			height: 300 * resoluteFrac
			visible: plotDisplay
			border.color: borderColors
			border.width: boarderWidths
			color: barColors
			ChartView {
				id: chart_line_throughput
				anchors.fill: parent
				animationOptions: ChartView.NoAnimation
				property bool openGL: true
				backgroundRoundness: 0
				legend.alignment: Qt.AlignBottom
				legend.font.pixelSize: resoluteFrac == 1 ? 12 : (resoluteFrac == 2 ? 24 : 9 * resoluteFrac)
				titleFont.pixelSize: 13* resoluteFrac
				legend.visible: true
				margins.bottom: 0
				margins.top: 45 * resoluteFrac
				margins.left: 10 * resoluteFrac
				plotAreaColor: "transparent"
				backgroundColor: "transparent"
				Rectangle{
					width: parent.width
					height: 35 * resoluteFrac
					color: barTitleColors
					anchors.top: parent.top
					anchors.topMargin: 0
					anchors.left: parent.left
					anchors.leftMargin: 0
					Label{
						text: "Memory Throughput (Gbps)"
						anchors.leftMargin: 20
						anchors.left: parent.left
						height: parent.height
						verticalAlignment: Text.AlignVCenter
						font.bold: true
						font.pointSize: 15 * resoluteFrac
						font.family: fontFamily.name
					}
				}
				ValueAxis {
					id: axisYtp
					min: 0
					max: 40
					labelFormat: "%d"
					labelsFont.pointSize: 10 * resoluteFrac
					labelsColor: "black"
					gridLineColor: "black"
					lineVisible: false
                                }
                                ValueAxis {
					id: axisXtp
					min: 0
					max: 60
					labelsVisible: false
					gridLineColor: "black"
					lineVisible: false
					labelsFont.pointSize: 1 * resoluteFrac
                                }
                                SplineSeries {
					name: "Video Source (00)"
					axisX: axisXtp
					axisY: axisYtp
					useOpenGL: chart_line_throughput.openGL
                                }
                                SplineSeries {
					name: "Accelerator (00)"
					axisX: axisXtp
					axisY: axisYtp
					useOpenGL: chart_line_throughput.openGL
                                }
                                SplineSeries {
					name: "Display (00)"
					axisX: axisXtp
					axisY: axisYtp
					useOpenGL: chart_line_throughput.openGL
                                }
			}
		}
		transform: Rotation {
			id: rotationMT
			origin.x: flipableMT.width/2
			origin.y: flipableMT.height/2
			axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
			angle: 0    // the default angle
		}
		states: State {
			name: "back"
			PropertyChanges { target: rotationMT; angle: 180 }
			when: flipableMT.flipped
                }

                transitions: Transition {
			NumberAnimation { target: rotationMT; property: "angle"; duration: 200 }
                }
                MouseArea {
			anchors.fill: parent
			hoverEnabled: true
			cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
			onClicked: flipableMT.flipped = !flipableMT.flipped

			onEntered:  {
				toolTipOnEnter(parent.x + parent.width/3,parent.y  +parent.height/2 + 10, "Click to Toggle View")
			}
			onExited: {
				toolTipOnExit()
			}
		}
                Behavior on height{
			NumberAnimation {
				duration: 500
				easing.type: Easing.OutSine
			}
		}
	}
	Rectangle{
		width: parent.width
		height: 100
		anchors.bottom: toolBarItem.top
		anchors.bottomMargin: 100 * resoluteFrac
		color: "transparent"
		MouseArea{
			hoverEnabled: true
			anchors.fill: parent
			onEntered: {
				showPlayarea(false)
				showDemoarea(false)
				showInputarea(false)
				showFilterarea(false)
				showFilterTypeArea(false)
			}
		}
	}
	Timer {
		id: refreshTimer
		interval: 1000
		running: true
		repeat: true
		onTriggered: {
			dataSource.updatecpu(chart_line_CPU.series(0),chart_line_CPU.series(1),chart_line_CPU.series(2),chart_line_CPU.series(3))
			if(showMemoryThroughput)
				dataSource.updateThroughput(chart_line_throughput.series(0),chart_line_throughput.series(1),chart_line_throughput.series(2))
		}
	}
	/* Source options display view */
	Rectangle{
		id: inputRectangle
		width: 100 * resoluteFrac
		anchors.leftMargin: (25 + 40 + 57) * resoluteFrac    // icon width + margin
		anchors.left: parent.left
		height: 0
		border.color: borderColors
		border.width: boarderWidths

		clip: true
		color: barColors
		anchors.bottom: toolBarItem.top
		anchors.bottomMargin: 1


		OptionsScrollVu{
			id: videoSrcOptionsSV
			anchors.fill: parent    // icon width + margin
			listModel.model: videoSourceList
			selectedItem: videoInput
			delgate: this
			width: parent.width
			function clicked(indexval){
				popupVisible = 0
				videoInput = indexval
				setInputType(videoSourceList[indexval].sourceIndex)
				setpreset(presetPatternSelected)
			}
		}
	}
	Rectangle{
		id: filterTypeRectangle
		width: 140 * resoluteFrac
		anchors.leftMargin: (25 + 37 + 37 + 25 + 57) * resoluteFrac    // icon width + margin
		anchors.left: parent.left
		height: 0
		border.color: borderColors
		border.width: boarderWidths

		clip: true
		color: barColors
		anchors.bottom: toolBarItem.top
		anchors.bottomMargin: 1


		OptionsScrollVu{
			id: filterTypeOptionsSV
			anchors.fill: parent    // icon width + margin
			listModel.model: filterTypeSourceList
			selectedItem: filterTypeInput
			delgate: this
			width: parent.width
			function clicked(indexval){
				popupVisible = 0
				if(play)
					filterTypeSettingsImage.source = (filterTypeSourcesCount && filterTypeSourceList[indexval].hasPanel) ?  "qrc:///imgs/images/gears@"+imageResolution+".png" : "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
				if((filterTypeSourceList.length ? filterTypeSourceList[indexval].displayName == "Optical Flow" : false)  && filterinput == 1){
					errorMessageText = "<b>SW</b> mode for <b>Optical Flow</b> is not supported"
					errorNameText = "Error"
					resetSource(filterTypeInput)
				}else{
					filterTypeInput = indexval
					setFilterType(filterTypeSourceList[indexval].sourceIndex)
					setpreset(presetPatternSelected)
					numericFilterTypeLabel.text = filterTypeSourceList.length ? filterTypeSourceList[filterTypeInput].displayName : ""
				}
			}
		}
	}
	Rectangle{
		id: sobleRectangle
		width: 100 * resoluteFrac
		anchors.leftMargin: (25 + 37 + 37 + 57 + 30 + 57) * resoluteFrac
		anchors.left: parent.left
		height: 0
		border.color: borderColors
		border.width: boarderWidths
		Behavior on height{
			NumberAnimation {
				duration: 0
				easing.type: Easing.Linear
			}
		}
		clip: true
		color: barColors
		anchors.bottom: toolBarItem.top
		anchors.bottomMargin: 1
		ExclusiveGroup { id: inputradExGroup }


		ColumnLayout{
			spacing: 0
			anchors.fill: parent
			id: inputFiltercombobox
			Rectangle{
				Text {
					text: filterModes[0]
					font.pointSize: 13 * resoluteFrac
					anchors.horizontalCenter: parent.horizontalCenter
					font.family: fontFamily.name
				}
				anchors.left: parent.left
				width: 100 * resoluteFrac
				height: childrenRect.height
				color: filterinput == 0 ? cellHighlightColor : cellColor
				MouseArea{
					anchors.fill: parent
					hoverEnabled: true
					cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
					onClicked:{
						filterinput = 0
						setFilter(0)
						numericFilterLabel.text = filterModes[filterinput]
						popupVisible = 0
					}
				}
			}
			Rectangle{
				Text {
					text: filterModes[1]
					font.family: fontFamily.name
					font.pointSize: 13 * resoluteFrac
					anchors.horizontalCenter: parent.horizontalCenter
				}
				anchors.left: parent.left
				width: 100 * resoluteFrac
				height: childrenRect.height
				color: filterinput == 1 ? cellHighlightColor : cellColor
				MouseArea{
					anchors.fill: parent
					hoverEnabled: true
					cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
					onClicked:{
						if(numericFilterTypeLabel.text == "Optical Flow"){
							errorMessageText = "<b>SW</b> mode for <b>Optical Flow</b> is not supported"
							errorNameText = "Error"
						}else{
							filterinput = 1
							setFilter(1)
							setpreset(presetPatternSelected)
							numericFilterLabel.text = filterModes[filterinput]
							popupVisible = 0
						}
					}
				}
			}
			Rectangle{
				Text {
					text: filterModes[2]
					font.family: fontFamily.name
					font.pointSize: 13 * resoluteFrac
					anchors.horizontalCenter: parent.horizontalCenter
				}
				anchors.left: parent.left
				width: 100 * resoluteFrac
				height: childrenRect.height
				color: filterinput == 2 ? cellHighlightColor : cellColor
				MouseArea{
					anchors.fill: parent
					hoverEnabled: true
					cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
					onClicked:{
						filterinput = 2
						setFilter(2)
						setpreset(presetPatternSelected)
						numericFilterLabel.text = filterModes[filterinput]
					}
				}
			}

		}
	}
	Rectangle{
		id: hideRectangle
		width: root.width - (290 * resoluteFrac)
		height: 100
		anchors.leftMargin: 10 * resoluteFrac
		anchors.left: sobleRectangle.right
		Behavior on height{
			NumberAnimation {
				duration: 200
				easing.type: Easing.OutBounce
			}
		}
		clip: true
		anchors.bottom: toolBarItem.top
		anchors.bottomMargin: 1
		color: "transparent"
		MouseArea{
			hoverEnabled: true
			anchors.fill: parent
			onEntered: {
				showPlayarea(false)
				showDemoarea(false)
				showInputarea(false)
				showFilterarea(false)
				showFilterTypeArea(false)
			}
		}
	}

	// bottom tool bar
	Rectangle{
		id: toolBarItem
		width: parent.width
		height: toolBarHeight
		anchors{
			bottom: parent.bottom
			bottomMargin: 20 * resoluteFrac
		}
		color: barColors
		Behavior on y{
			NumberAnimation {
				duration: 200
				easing.type: Easing.OutBounce
			}
		}
		RowLayout{

			Rectangle{
				height: childrenRect.height
				clip: true
				anchors.topMargin: 0
				anchors.leftMargin: 0
				anchors.top: parent.top
				anchors.left: parent.left
				color: "transparent"
				Behavior on width{
					NumberAnimation {
						duration: 200
					}
				}
				id: playBtnArea
				width: playButton.width
//				RowLayout{
					Button{
						id: playButton
						width: toolBarHeight
						height: toolBarHeight
						anchors.topMargin: 0
						anchors.leftMargin: 0
						anchors.top: parent.top
						anchors.left: parent.left
						Image {
							anchors.fill: playButton
							source: (!play ? "qrc:///imgs/images/play@"+imageResolution+".png" : "qrc:///imgs/images/pause@"+imageResolution+".png")
						}
						style: ButtonStyle{
							background: Rectangle{
								color: "transparent"
							}
						}

						MouseArea{
							anchors.fill: parent
							hoverEnabled: true
							cursorShape: containsMouse ? (auto_start ? Qt.ArrowCursor:Qt.PointingHandCursor) : Qt.ArrowCursor
							onEntered: {
								//show other buttons
								showPlayarea(true)
							}
							onPositionChanged: {
								if(!auto_start)
									toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x + 10,toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Start/Stop Manual Mode")
							}
							onExited: {
								toolTipOnExit()
							}
							onClicked: {
								if(auto_start){
									auto_start = !auto_start
									filterinput = 0
									videoInput = 0
									filterTypeInput = 0
									setFilter(0)
									numericInputLabel.text = videoSourceList.length?videoSourceList[videoInput].shortName:"No Video Sources Available"
									numericFilterTypeLabel.text = filterTypeSourceList.length ? filterTypeSourceList[filterTypeInput].displayName : ""
									numericFilterLabel.text = filterModes[filterinput]
									videoSrcOptionsSV.resetSource(videoInput)
									filterTypeOptionsSV.resetSource(filterTypeInput)
									filterTypeSettingsImage.source =  "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
									sourceSettingsButtonImage.source = "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
									demoMode(auto_start)
									presetPatternSelected = 1
								}
								if(play){
									play = false
									root.playvideo(0)
									filterTypeSettingsImage.source =  "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
									sourceSettingsButtonImage.source = "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
								}else{
									play = true
									setInputType(videoSourceList[videoInput].sourceIndex)
									if(filterTypeSourcesCount)
										setFilterType(filterTypeSourceList[filterTypeInput].sourceIndex)
									numericFilterLabel.text = filterModes[filterinput]
									root.playvideo(1)
									setpreset(presetPatternSelected)
									filterTypeSettingsImage.source = (filterTypeSourcesCount && filterTypeSourceList[filterTypeInput].hasPanel) ?  "qrc:///imgs/images/gears@"+imageResolution+".png" : "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
									sourceSettingsButtonImage.source = videoSourceList[videoInput].hasPanel ? "qrc:///imgs/images/gears@"+imageResolution+".png" : "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
								}
							}
						}
					}
				}
				// rectangle vertical line
				Rectangle{
					id: demoAreaSeperator
					width: 2 * resoluteFrac
					anchors{
						left: playBtnArea.right
					}
					color: "transparent"
					Image {
						anchors.fill: parent
						source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
					}
					height: toolBarHeight
				}
				Rectangle{
					height: childrenRect.height
					clip: true
					anchors.topMargin: 0
					anchors.leftMargin: 0
					anchors.top: parent.top
					anchors.left: demoAreaSeperator.right
					color: "transparent"
					Behavior on width{
						NumberAnimation {
							duration: 200
						}
					}
					id: demoBtnArea
					width: demoButton.width
					Button{
						id: demoButton
						anchors.topMargin: 0
						anchors.leftMargin: 0
						anchors.top: parent.top
						width: toolBarHeight
						height: toolBarHeight
						anchors.left: parent.left
						Image {
							anchors.fill: parent
							source: auto_start ? "qrc:///imgs/images/pause@"+imageResolution+".png" : "qrc:///imgs/images/repeat@"+imageResolution+".png"
						}
						MouseArea{
							anchors.fill: parent
							hoverEnabled: true
							cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
							onPositionChanged: {
								onEntered: {
									//show other buttons
									showDemoarea(true)
								}
								toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x + 10,toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Start/Stop Demo Mode")
							}
							onExited: {
								toolTipOnExit()
							}
							onClicked:{
								auto_start = !auto_start
								if(auto_start){
							    // stop play if it is playing
									play = false
									if(play)
										root.playvideo(0)

								}else{
									filterinput = 0
									videoInput = 0
									filterTypeInput = 0
									setFilter(0)
									numericInputLabel.text = videoSourceList.length?videoSourceList[videoInput].shortName:"No Video Sources Available"
									numericFilterTypeLabel.text = filterTypeSourceList.length ? filterTypeSourceList[filterTypeInput].displayName : ""
									numericFilterLabel.text = filterModes[filterinput]
									videoSrcOptionsSV.resetSource(videoInput)
									filterTypeOptionsSV.resetSource(filterTypeInput)
								}
								filterTypeSettingsImage.source =  "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
								sourceSettingsButtonImage.source = "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
								demoMode(auto_start)
								presetPatternSelected = 1
							}
						}
						style: ButtonStyle{
							background: Rectangle{
								color: "transparent"
							}
						}

					}
					Button{
						Image {
							id: demoSettingsButtonImage
							anchors.fill: parent
							source: !play ? "qrc:///imgs/images/gears@"+imageResolution+".png" : "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
						}
						id: demoPopSettingsButton
						width: toolBarHeight
						height: toolBarHeight
						anchors.left: demoButton.right
						enabled: true
						style: ButtonStyle{
							background: Rectangle{
								color: "transparent"
							}
						}
						MouseArea{
							anchors.fill: parent
							hoverEnabled: true
							cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
							onPositionChanged: {
								toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x + 10,toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Show Demo Mode Settings")
							}
							onExited: {
								toolTipOnExit()
							}
							onClicked: {
								if(!play){
									popupVisible = "Demo Mode"
									popupTitle = "Demo Mode Settings"
								}
							}
						}
					}
//				}
			}
			// rectangle vertical line
			Rectangle{
				id: playAreaSeperator
				width: 2 * resoluteFrac
				anchors{
					left: demoBtnArea.right
//					leftMargin: 8 * resoluteFrac
				}
				color: "transparent"
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
				}

				height: toolBarHeight
			}

			Rectangle{
				height: childrenRect.height
				clip: true
				anchors{
//                    leftMargin: 2 * resoluteFrac
				}
				color: "transparent"

				Behavior on width{
					NumberAnimation {
						duration: 200
					}
				}
				id: inputBtnArea
				width: inputButton.width
				anchors{
				left: playAreaSeperator.right
				}
//				RowLayout{
					Button{
						id: inputButton
						width: toolBarHeight
						height: toolBarHeight
						Image {
							anchors.fill: parent
							source: auto_start ? "qrc:///imgs/images/configurationDisabled@"+imageResolution+".png" : "qrc:///imgs/images/configuration@"+imageResolution+".png"
						}
						style: ButtonStyle{
							background: Rectangle{
								color: "transparent"
							}
						}

						MouseArea{
							anchors.fill: parent
							hoverEnabled: true
							cursorShape: containsMouse ? (auto_start ? Qt.ArrowCursor :Qt.PointingHandCursor) : Qt.ArrowCursor
							onEntered: {
								if(!auto_start){
									showInputarea(true)
									inputRectangle.height = 22 * videoSourcesCount * resoluteFrac
								}
							}
							onPositionChanged: {
								if(!auto_start)
									toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x + (110 * resoluteFrac),toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Select Video Source")
							}
							onExited: {
								toolTipOnExit()
							}
						}
					}
					Button{
						Image {
							id: sourceSettingsButtonImage
							anchors.fill: parent
							source: "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
						}
						id: inputPopSettingsButton
						width: toolBarHeight
						height: toolBarHeight
						anchors.left: inputButton.right
						enabled: true
						style: ButtonStyle{
							background: Rectangle{
								color: "transparent"
							}
						}
						MouseArea{
							anchors.fill: parent
							hoverEnabled: true
							cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
							onPositionChanged: {
								toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x  + (50 * resoluteFrac),toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Show Video Source Settings")
							}
							onExited: {
								toolTipOnExit()
							}
							onClicked: {
								if(play && videoSourceList[videoInput].hasPanel && !auto_start){
									popupVisible = videoSourceList[videoInput].settingsPanelName
									popupTitle = videoSourceList[videoInput].displayName + " Settings"
								}
							}
						}
					}

//				}

			}
			// rectangle vertical line
			Rectangle{
				id: inputButnAreaDivider
				width: 2 * resoluteFrac
				anchors{
					left: inputBtnArea.right
//					leftMargin: 8 * resoluteFrac
				}
				color: "transparent"
				height: toolBarHeight
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
				}
			}

			Rectangle{
				visible: filter2DEnabled
				height: childrenRect.height
				clip: true
				anchors{
//					leftMargin: 15 * resoluteFrac
				}
				color: "transparent"

				Behavior on width{
					NumberAnimation {
						duration: 200
					}
				}
				id: filterTypeArea
				width: inputButton.width
				anchors{
					left: inputButnAreaDivider.right
				}

				Button{
					width: toolBarHeight
					height: toolBarHeight
					id: filterTypeButton

					Image {
						anchors.fill: parent
						source: auto_start ? "qrc:///imgs/images/filterTypeDisabled@"+imageResolution+".png" :"qrc:///imgs/images/filterType@"+imageResolution+".png"
					}
					style: ButtonStyle{
						background: Rectangle{
							color: "transparent"
						}
					}

					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? (auto_start ? Qt.ArrowCursor : Qt.PointingHandCursor) : Qt.ArrowCursor
						onPositionChanged: {
							if(!auto_start)
								toolTipOnEnter(toolBarItem.x + parent.parent.x + parent.x + (140 * resoluteFrac), toolBarItem.y + parent.parent.y + parent.y - (20 * resoluteFrac), "Select Accelerator Type")
						}
						onExited: {
							toolTipOnExit()
						}
						onEntered: {
							if(!auto_start){
								showFilterTypeArea(true)
								filterTypeRectangle.height = 22 * filterTypeSourcesCount * resoluteFrac
							}
						}
					}
				}
				Button{
					Image {
						id: filterTypeSettingsImage
						anchors.fill: parent
						source: "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
					}
					width: toolBarHeight
					height: toolBarHeight
					anchors.left: filterTypeButton.right
					id:sobelpopUpSettingsButton

					style: ButtonStyle{
						background: Rectangle{
							color: "transparent"
						}
					}
					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
						onPositionChanged: {
							toolTipOnEnter(toolBarItem.x + parent.parent.x + parent.x + (90 * resoluteFrac),toolBarItem.y + parent.parent.y + parent.y  - (20 * resoluteFrac), "Show Accelerator Settings")
						}
						onExited: {
							toolTipOnExit()
						}
						onClicked: {
							if(play && (filterTypeSourcesCount && filterTypeSourceList[filterTypeInput].hasPanel) && !auto_start){
								popupVisible = filterTypeSourceList[filterTypeInput].settingsPanelName
								popupTitle = filterTypeSourceList[filterTypeInput].displayName + " Settings"
							}
						}
					}
				}
			}
			// rectangle vertical line
			Rectangle{
				visible: filter2DEnabled
				width: 2 * resoluteFrac
				id:filterTypeBtnVertLine
				anchors{
					left: filterTypeArea.right
				}
				color: "transparent"
				height: toolBarHeight
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
				}
			}
			Rectangle{
				visible: filter2DEnabled
				height: childrenRect.height
				clip: true
				color: "transparent"

				Behavior on width{
					NumberAnimation {
						duration: 200
					}
				}
				id: filterBtnArea
				width: inputButton.width
				anchors{
					left: filterTypeBtnVertLine.right
				}

				Button{
					width: toolBarHeight
					height: toolBarHeight
					id: filterButton

					Image {
						anchors.fill: parent
						source: auto_start ? "qrc:///imgs/images/filterModeDisabled@"+imageResolution+".png" :"qrc:///imgs/images/filterMode@"+imageResolution+".png"
					}
					style: ButtonStyle{
						background: Rectangle{
							color: "transparent"
						}
					}

					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? (auto_start ? Qt.ArrowCursor :Qt.PointingHandCursor) : Qt.ArrowCursor
						onPositionChanged: {
							if(!auto_start)
								toolTipOnEnter(toolBarItem.x + parent.parent.x + parent.x + (110 * resoluteFrac),toolBarItem.y + parent.parent.y + parent.y  - (20 * resoluteFrac), "Select Accelerator Mode")
						}
						onExited: {
							toolTipOnExit()
						}
						onEntered: {
							if(play){
								showFilterarea(true)
								sobleRectangle.height = 22 * 3 * resoluteFrac
							}
						}
					}
				}


			}
			// rectangle vertical line
			Rectangle{
				visible: filter2DEnabled
				width: 2 * resoluteFrac
				id:filterBtnVertLine
				anchors{
					left: filterBtnArea.right
//					leftMargin: 8 * resoluteFrac
				}
				color: "transparent"
				height: toolBarHeight
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
				}
			}
			Rectangle{
				width: 100 * resoluteFrac
				anchors{
					left: filterBtnVertLine.right
					leftMargin: 8 * resoluteFrac
				}
				color: "transparent"
				height: toolBarHeight
				MouseArea{
					hoverEnabled: true
					anchors.fill: parent
					onEntered: {
						showPlayarea(false)
						showDemoarea(false)
						showInputarea(false)
						showFilterarea(false)
						showFilterTypeArea(false)
					}
				}
			}



		}
		// close settings and other buttons
		Rectangle{
			id: hidePanel
			width: 100
			height: parent.height
			color: "transparent"
			anchors.right: displayOptionsPanel.left
			MouseArea{
				hoverEnabled: true
				anchors.fill: parent
				onEntered: {
				}
			}
		}

		Rectangle{
			Behavior on width{
				NumberAnimation {
					duration: 200
				}
			}
			clip: true
			id: displayOptionsPanel
			color: "transparent"
			height: toolBarHeight
			width: (2 * toolBarHeight) + (4 * resoluteFrac)
			anchors.right: controlBarPin.left
//			RowLayout{
				Rectangle{
					width: 2 * resoluteFrac
					anchors{
						right: displayNumPanelCheckBx.left
//						rightMargin: 1 * resoluteFrac

					}
					color: "transparent"
					height: toolBarHeight
					Image {
						anchors.fill: parent
						source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
					}

				}
				Rectangle{

					Image {
						id: numericPanelCheckImage
						source: statsDisplay ? "qrc:///imgs/images/numStatsOn@"+imageResolution+".png" : "qrc:///imgs/images/numStats@"+imageResolution+".png"
						anchors.fill: parent
//						anchors.verticalCenter: parent.verticalCenter
//						anchors.left: parent.left
//						anchors.leftMargin: 5
					}
					id: displayNumPanelCheckBx
					anchors.right: numstatsSepLine.left
//					anchors.leftMargin: 4 * resoluteFrac
					width: toolBarHeight
					height: toolBarHeight
					color: "transparent"
					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
						onPositionChanged: {
							toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x - (tooltipWidth),toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Show/Hide Video Info")
						}
						onExited: {
							toolTipOnExit()
						}
						onClicked:{
							statsDisplay = !statsDisplay
						}
					}

				}
				Rectangle{
					id: numstatsSepLine
					width: 2 * resoluteFrac
					anchors{
						right: displayPlotsPanelCheckBx.left
//						leftMargin: 4 * resoluteFrac
					}
					color: "transparent"
					height: toolBarHeight
					Image {
						anchors.fill: parent
						source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
					}
				}
				Rectangle{

					Image {
						id: plotsPanelCheckImage
						source: plotDisplay ? "qrc:///imgs/images/statOn@"+imageResolution+".png" : "qrc:///imgs/images/stat@"+imageResolution+".png"
						anchors.fill: parent
					}
					id: displayPlotsPanelCheckBx
					anchors.right: parent.right
//					anchors.leftMargin: 8 * resoluteFrac
					width: toolBarHeight
					height: toolBarHeight
					color: "transparent"
					MouseArea{
						anchors.fill: parent
						hoverEnabled: true
						cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
						onPositionChanged: {
							toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x - (tooltipWidth),toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Show/Hide Performance Monitor")
						}
						onExited: {
							toolTipOnExit()
						}
						onClicked:{
							plotDisplay = !plotDisplay
						}
					}

				}
//			}
		}
		Rectangle{
			id: controlBarPin
			color: "transparent"
			height: toolBarHeight
			width: (toolBarHeight)+(2 * resoluteFrac)
			anchors.right: closeButton.left
			Rectangle{
				width: 2 * resoluteFrac
				anchors{
					right: pinIcon.left
//					rightMargin: 1 * resoluteFrac
				}
				color: "transparent"
				height: toolBarHeight
				Image {
					anchors.centerIn: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
					anchors.fill: parent
				}

			}
			Button{
				id: pinIcon
				height: toolBarHeight
				width: toolBarHeight
				anchors.right: parent.right
				Image {
					anchors.centerIn: parent
					source: hideControlBar ? "qrc:///imgs/images/pin@"+imageResolution+".png" : "qrc:///imgs/images/unpin@"+imageResolution+".png"
					anchors.fill: parent
				}
				style: ButtonStyle{
					background: Rectangle{
						color: "transparent"
					}
				}
				MouseArea{
					anchors.fill: parent
					hoverEnabled: true
					cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
					onPositionChanged: {
						toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x  - (tooltipWidth),toolBarItem.y +parent.parent.y + parent.y  - (20 * resoluteFrac), "Pin/Unpin Control Bar")
					}
					onExited: {
						toolTipOnExit()
					}
					onClicked:{
						hideControlBar = !hideControlBar

					}
				}
			}
		}

		Rectangle{
			Rectangle{
				width: 2 * resoluteFrac
				anchors{
					left: closeButton.left
				}
				color: "transparent"
				height: toolBarHeight
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/icon-divider@"+imageResolution+".png"
				}
			}
			color: "transparent"
			width: parent.height + (2 * resoluteFrac)
			height: parent.height
			anchors.right: parent.right
			id: closeButton
			Timer{
				id: quitTimer
				repeat: false
				interval: 5
				running: false
				onTriggered: {
				Qt.quit()
				}
			}
			Button{
				height: toolBarHeight
				width: toolBarHeight
				anchors.right: parent.right
				Image {
					anchors.fill: parent
					source: "qrc:///imgs/images/close@"+imageResolution+".png"
				}
				style: ButtonStyle{
					background: Rectangle{
						color: "transparent"
					}
				}
				MouseArea{
					id: exitClickedMA
					anchors.fill: parent
					hoverEnabled: true
					cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
					onPositionChanged: {
						toolTipOnEnter(toolBarItem.x +parent.parent.x + parent.x  - (tooltipWidth),toolBarItem.y + parent.y - (20 * resoluteFrac), "Exit Application")
					}
					onExited: {
						toolTipOnExit()
					}
					onClicked:{
						errorMessageText = ""
						errorNameText = ""
						exitClickedMA.cursorShape = Qt.BlankCursor
						refreshTimer.stop()
						// removing other components
						appRunning = false
						popupVisible = 0
						toolBarItem.visible = false
						numericPanel.visible = false
						cpuUtilizationPlot.visible = false
						flipableMT.visible = false
						flipable.visible = false
						quitTimer.start()
					}
				}

			}

		}


	}
	/*Error message*/
	ErrorMessage{
		width: parent.width
		height: parent.height
		id: errorMessage
		messageText: errorMessageText
		errorName: errorNameText
		message.onClicked:{
			errorMessageText = ""
			errorNameText = ""
		}
	}
	/*Tool tip rectangl*/
	Tooltip {
		id: toolTipRect
		delegate: this
		function tooltipWdth(wdth){
			tooltipWidth = wdth;
		}
	}

	// Supported functions

	function showPlayarea(val){
		if(val == true){
			// hide other open menus here
			playBtnArea.width = playBtnArea.childrenRect.width
			showDemoarea(false)
			showInputarea(false)
			showFilterarea(false)
			showFilterTypeArea(false)
			inputRectangle.height = 0
			sobleRectangle.height = 0
			filterTypeRectangle.height = 0
			popupVisible = 0
		}else{
			playBtnArea.width = playButton.width
		}
	}
	function showDemoarea(val){
		if(val == true){
			// hide other open menus here
			demoBtnArea.width = demoBtnArea.childrenRect.width
			showPlayarea(false)
			showInputarea(false)
			showFilterarea(false)
			showFilterTypeArea(false)
			inputRectangle.height = 0
			sobleRectangle.height = 0
			filterTypeRectangle.height = 0
			popupVisible = 0
		}else{
			demoBtnArea.width = demoButton.width
		}
	}
	function showInputarea(val){
		if(val == true){
			showPlayarea(false)
			showDemoarea(false)
			showFilterarea(false)
			showFilterTypeArea(false)
			inputBtnArea.width = inputBtnArea.childrenRect.width
			popupVisible = 0
		}else{
			inputBtnArea.width = inputButton.width
		}
		sobleRectangle.height = 0
	}
	function showFilterarea(val){
		if(val == true){
			showPlayarea(false)
			showInputarea(false)
			showDemoarea(false)
			showFilterTypeArea(false)
			filterBtnArea.width = filterBtnArea.childrenRect.width
			popupVisible = 0
		}else{
			filterBtnArea.width = filterButton.width
		}
		inputRectangle.height = 0
	}
	function showFilterTypeArea(val){
		if(val == true){
			showPlayarea(false)
			showInputarea(false)
			showDemoarea(false)
			showFilterarea(false)
			filterTypeArea.width = filterTypeArea.childrenRect.width
			popupVisible = 0
		}else{
			filterTypeArea.width = filterTypeButton.width
		}
		filterTypeRectangle.height = 0
	}
	function applyCoeff(){
		filterCoeff(filterCoeffList[filter00Coeff],filterCoeffList[filter01Coeff],filterCoeffList[filter02Coeff],
			filterCoeffList[filter10Coeff],filterCoeffList[filter11Coeff],filterCoeffList[filter12Coeff],
			filterCoeffList[filter20Coeff],filterCoeffList[filter21Coeff],filterCoeffList[filter22Coeff])
	}
	function abortDemoMode(val){
		filterinput = 0
		videoInput = 0
		filterTypeInput = 0
		setFilter(0)
		numericInputLabel.text = videoSourceList.length?videoSourceList[videoInput].shortName:"No Video Sources Available"
		numericFilterTypeLabel.text = filterTypeSourceList.length ? filterTypeSourceList[filterTypeInput].displayName : ""
		numericFilterLabel.text = filterModes[filterinput]
		videoSrcOptionsSV.resetSource(videoInput)
		filterTypeOptionsSV.resetSource(filterTypeInput)
		filterTypeSettingsImage.source =  "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
		sourceSettingsButtonImage.source = "qrc:///imgs/images/gearsDisabled@"+imageResolution+".png"
		presetPatternSelected = 1
	}
}
