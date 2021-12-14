/////////////////////////////////////////////////////////////////////////////////
// WebPages.h
//
// Contains the (very long) string that represents the root web page for the
// filament scale.
//
// History:
// - jmcorbett 27-JUN-2021 Original creation.
//
// Copyright (c) 2021, Joseph M. Corbett
/////////////////////////////////////////////////////////////////////////////////
#if !defined WEBPAGES_H
#define WEBPAGES_H

const char gRootPage[] = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>JMC Filament Scale</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <link rel="stylesheet" href="https://www.w3schools.com/lib/w3-theme-blue-grey.css">
  </head>
  <style>

    a:hover, a:active, .spoolLinkClass:hover, filamentDensityClass:hover { color: #c0c0c0; }

    .form-popup, #idWorking {
      display: none;
      position: fixed;
      top: 10%;
      right: 15px;
      z-index: 9;
    }

    /* Add styles to the form container */
    .form-container, #idWorking {
      width: 300px;
      padding: 15px;
    }

    /* Full-width input fields */
    .form-container input[type=number], .form-container input[type=text], .w3-select {
      width: 100%;
      padding: 5px;
      margin: 1px 0 15px 0;
    }

    /* When the inputs get focus, do something */
    .form-container input[type=text]:focus, .form-container input[type=number]:focus {
      background-color: #ddd;
      outline: none;
    }

    /* Set a style for the submit/login button */
    .form-container .btn {
      color: white;
      padding: 16px 20px;
      cursor: pointer;
      width: 50%;
      margin-bottom:10px;
      opacity: 0.8;
    }

    /* Add a red background color to the cancel button */
    .form-container .cancel { background-color: crimson; }

    /* Add some hover effects to buttons */
    .form-container .btn:hover, .open-button:hover { opacity: 1; }

    /* Customized alert box. */
    .my-alert {
      height:60%;
      min-height:450px;
      z-index:10;
      display:none;
    }

    /* Custom alert box text. */
    .my-alert-text {
      font-size:x-large;
      width:90%;
      margin-left:auto;
      margin-right:auto;
      text-align:center;
      position:absolute;
      top:50%;
      transform:translate(0%, -50%);
    }

  </style>


  <body class="w3-theme-d1">

    <!-- HEADER -->
    <div class="w3-col" style="width:25%">&nbsp</div>
    <div class="w3-container w3-col" style="width:50%">
      <div class="w3-card-4 w3-center w3-black w3-round-xlarge w3-border">
        <h1>JMC Filament Scale</h1>
        <h5 id="idWebId"></h5>
      </div>
    </div>

    <!-- MENU BAR -->
    <div class="w3-bar w3-theme-l2 w3-round-large" style="width:90%;position:relative;left:5%;top:10px;">
      <a href="#" onclick="getDisplayFormData()" class="w3-bar-item w3-button w3-mobile">Display</a>
      <a href="#" onclick="getScaleFormData()" class="w3-bar-item w3-button w3-mobile">Scale</a>
      <a href="#" onclick="getSpoolFormData()" class="w3-bar-item w3-button w3-mobile">Spool</a>
      <a href="#" onclick="getDensityFormData()" class="w3-bar-item w3-button w3-mobile">Filament</a>
      <a href="#" onclick="startSaveForm()" class="w3-bar-item w3-button w3-mobile">Save/Restore</a>
    </div>

    <!-- SCALE DATA -->
    <div class="w3-container" style="position:relative;top:16px;">
      <fieldset class="w3-container w3-round-xlarge w3-card-4 w3-theme-d2" id="idScaleDataFieldset">
        <legend>Scale Data</legend>
        <br>

        <div class="w3-row">

          <!-- NET WEIGHT -->
          <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:210px;" id="idNetWtContainer">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Net Weight</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Tare
                </span>
                <a href="#" onclick="doTare()" style="text-decoration: none;">
                  <div id="idNetWeight" style="width: 100%;"></div>
                </a>
              </div>
            </div>
          </div>

          <!-- FILAMENT LENGTH -->
          <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:310px;" id="idLengthContainer">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Filament Length</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Change Units
                </span>
                <a href="#" onclick="getDisplayFormData()" style="text-decoration: none;">
                  <div id="idLength" class="w3-center w3-xxlarge"></div>
                </a>
              </div>
            </div>
          </div>

          <!-- GROSS WEIGHT -->
          <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:210px;"  id="idGrossWtContainer">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Gross Weight</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Tare
                </span>
                <a href="#" onclick="doTare()" style="text-decoration: none;">
                  <div id="idGrossWeight" style="width: 100%;"></div>
                </a>
              </div>
            </div>
          </div>

        </div>
      <br>
      </fieldset>
    </div>

    <br>

    <!-- SPOOL / FILAMENT DATA -->
    <div class="w3-container" id="idSpoolContainer" style="position:relative; top:8px;">
      <fieldset class="w3-container w3-round-xlarge w3-card-4 w3-theme-d2">
        <legend>Spool/Filament</legend>
        <br>

        <div class="w3-row">

          <!-- SPOOL ID -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:170px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Spool ID</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                </span>
                <div id="idSpoolId" onclick="getSpoolFormData()" style="cursor:pointer" class="spoolLinkClass w3-center w3-xlarge"></div>
              </div>
            </div>
          </div>

          <!-- SPOOL WEIGHT -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:170px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Spool Weight</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                </span>
                <div id="idSpoolWeight" onclick="getSpoolFormData()" style="cursor:pointer"  class="spoolLinkClass w3-center w3-xlarge"></div>
              </div>
            </div>
          </div>

          <!-- FILAMENT COLOR -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:170px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Filament Color</p>
              <div id="idFilamentColor" class="w3-tooltip w3-center w3-xlarge w3-round-large w3-card-4 ">
                  <span style="position:absolute;left:25%;bottom:80%"
                    class="w3-card-4 w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                  </span>
                  <a href="#" onclick="getSpoolFormData()" style="text-decoration: none;" class="w3-card-4">
                    <div style="width: 100%;">&nbsp</div>
                  </a>
              </div>
            </div>
          </div>

          <!-- FILAMENT TYPE -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:170px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Filament Type</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                </span>
                <div id="idFilamentType" onclick="getSpoolFormData()" style="cursor:pointer"  class="spoolLinkClass w3-center w3-xlarge"></div>
              </div>
            </div>
          </div>

          <!-- FILAMENT DENSITY -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:180px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p id="idDensityLabel"></p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                </span>
                <div id="idDensity" onclick="getSpoolFormData()" style="cursor:pointer"  class="filamentDensityClass w3-center w3-xlarge"></div>
              </div>
            </div>
          </div>

          <!-- FILAMENT DIAMETER -->
          <div class="w3-col w3-container w3-padding-small" style="width:16.6%; min-width:180px;">
            <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
              <p>Filament Diameter</p>
              <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                <span style="position:absolute;left:40%;bottom:80%"
                  class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Modify
                </span>
                <div id="idFilamentDiameter" onclick="getSpoolFormData()" style="cursor:pointer"  class="spoolLinkClass w3-center w3-xlarge"></div>
              </div>
            </div>
          </div>

        </div>
      <br>
      </fieldset>
    </div>

    <br>

    <div class="w3-row">

      <!-- ENVIRONMENTAL DATA -->
      <div class="w3-col w3-container" style="width:50%;">
        <fieldset class="w3-container w3-round-xlarge w3-card-4 w3-theme-d2">
          <legend>Environmental</legend>
          <br>

          <div class="w3-row">

            <!-- TEMPERATURE -->
            <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:130px;">
              <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
                <p>Temperature</p>
                <div class="w3-tooltip w3-center w3-xxlarge w3-round-large">
                  <span style="position:absolute;left:40%;bottom:80%"
                    class="w3-text w3-tiny w3-tag w3-round-xlarge w3-animate-opacity">Change Units
                  </span>
                  <a href="#" onclick="getDisplayFormData()" style="text-decoration: none;">
                    <div id="idTemperature" class="w3-center w3-xlarge"></div>
                  </a>
                </div>
              </div>
            </div>

            <!-- HUMIDITY -->
            <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:120px;">
              <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
                <p >Humidity</p>
                <div id="idHumidity" class="w3-center w3-xlarge"></div>
              </div>
            </div>

            <!-- UP TIME -->
            <div class="w3-col w3-container w3-padding-small" style="width:33%; min-width:170px;">
              <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
                <p>Up Time</p>
                <div id="idUptime" class="w3-center w3-xlarge"></div>
              </div>
            </div>

          </div>
        <br>
        </fieldset>
      </div>

      <!-- NETWORK DATA -->
      <div class="w3-col w3-container" style="width:50%">
        <fieldset class="w3-container w3-round-xlarge w3-card-4 w3-theme-d2">
          <legend>Network</legend>
          <br>
          <div class="w3-row">

            <!-- IP ADDRESS -->
            <div class="w3-col w3-container w3-padding-small" style="width:50%; min-width:240px;">
              <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
                <p>IP Address</p>
                <div id="idIpAddress" class="w3-center w3-xlarge"></div>
              </div>
            </div>

            <!-- SIGNAL STRENGTH -->
            <div class="w3-col w3-container w3-padding-small" style="width:50%; min-width:160px;">
              <div class="w3-border w3-responsive w3-theme-d4 w3-card-4 w3-round-xlarge w3-padding-small">
                <p>Signal Strength</p>
                <div class="w3-dark-gray w3-round-large w3-card-4">
                  <div class="w3-card-4 w3-container w3-center w3-round-large w3-xlarge" id="idSignalStrength"</div>
                </div>
              </div>
            </div>
          <br>
          </div>
      </div>

    </div>


    <!-- DISPLAY OPTIONS FORM -->
    <div class="form-popup" id="idDisplayForm">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-theme-l2 w3-round-large w3-card">
        <h1>Display Options</h1>

        <label for="idWeightUnitsData"><b>Weight Units</b></label>
        <select class="w3-select w3-round-large w3-card" id="idWeightUnitsData" name="weightUnitsData" required>
          <option value="0">g</option>
          <option value="1">kg</option>
          <option value="2">oz</option>
          <option value="3">lb</option>
        </select>

        <label for="idLengthUnitsData"><b>Length Units</b></label>
        <select class="w3-select w3-round-large w3-card" id="idLengthUnitsData" name="lengthUnitsData" required>
          <option value="0">mm</option>
          <option value="1">cm</option>
          <option value="2">m</option>
          <option value="3">in</option>
          <option value="4">ft</option>
          <option value="5">yd</option>
        </select>

        <label for="idTempUnitsData"><b>Temperature Units</b></label>
        <select class="w3-select w3-round-large w3-card" id="idTempUnitsData" name="tempUnitsData" required>
          <option value="0">&deg;F</option>
          <option value="1">&deg;C</option>
        </select>

        <div style="padding:0 0 16px 0">
          <label for="brightnessData"><b id="idBrightnessLbl"></b></label>
          <input type="range" id="idBrightnessData" name="brightnessData" style="width:100%" min="5" max="100" step="5" class="w3-round-large w3-card" required>
        </div>

        <div style="padding:0 0 16px 0">
          <label for="scrollDelayData"><b id="idScrollDelayLbl"></b></label>
          <input type="range" id="idScrollDelayData" name="scrollDelayData" style="width:100%" min="0" max="120" step="5" class="w3-round-large w3-card" required>
        </div>

        <button type="submit" style="width:48%;" class="w3-button w3-round-large w3-card w3-teal" onclick="putDisplayFormData()">Update</button>
        <button type="button" style="width:48%;" class="w3-button cancel w3-round-large w3-card" onclick="unlockDisplayForm()">Cancel</button>
      </form>
    </div>


    <!-- SCALE OPTIONS FORM -->
    <div class="form-popup" id="idScaleForm">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-theme-l2 w3-round-large w3-card">
        <h1>Scale Options</h1>

        <button type="button" style="width:48%;" class="w3-button w3-round-large w3-card w3-theme-d1" onclick="doOptionsTare()">Tare</button>

        <div style="padding:16px 0 16px 0">
          <fieldset class="w3-container w3-round-xlarge w3-card-4 w3-theme-l2" id="idScaleFieldset">
            <legend>Calibration</legend>
            <div id="idScaleCalibrateWeightLbl">Weight (g)</div>
            <div style="width:100%;padding:0px 0 0 0">
              <input type="number" value="250.0" id="idScaleCalibrateWeightData" name="scaleCalibrateWeightData" min="0" max="5000" step="0.1" class="w3-round-large w3-card" style="width:48%" required>
              <button type="button" style="width:48%;" class="w3-button w3-round-large w3-card w3-theme-d1" onclick="doScaleCalibrate()">Calibrate</button>
            </div>
          </fieldset>
        </div>

        <label for="averageData"><b>Average Samples</b></label>
        <input type="number" name="averageData" id="idAverageSamples" value="4" min="1" max="10" step="1" class="w3-round-large w3-card" required>

        <label for="idScaleGainData"><b>Load Cell Gain</b></label>
        <select class="w3-select w3-round-large w3-card" id="idScaleGainData" name="scaleGainData" required>
          <option value="64">64</option>
          <option value="128">128</option>
        </select>

        <button type="submit" style="width:48%;" class="w3-button w3-round-large w3-card w3-teal" onclick="putScaleFormData()">Update</button>
        <button type="button" style="width:48%;" class="w3-button cancel w3-round-large w3-card" onclick="unlockScaleForm()">Cancel</button>
      </form>
    </div>


    <!-- SPOOL OPTIONS FORM -->
    <div class="form-popup" id="idSpoolForm">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-theme-l2 w3-round-large w3-card">
        <h1>Spool Data</h1>

        <label for="spoolIdData"><b>Use &nbsp Spool ID</b></label> <br>
        <input class="w3-check" type="checkbox" id="idSpoolSelect" style="width:12%">
        <input type="text" id="idSpoolIdData" name="spoolIdData" maxlength="8" class="w3-round-large w3-card" style="width:51%" value="AZZZZZZZZZZA" required>
        <button class="w3-button w3-circle" style="width:15%; background-color:#336600; color:#ffffff"" id="incSpool" onclick="incrementSpool()">&#8681</button>
        <button class="w3-button w3-circle" style="width:15%; background-color:#990000; color:#ffffff" id="decSpool" onclick="decrementSpool()">&#8679</button>

        <div>
          <label for="spoolColor"><b>Color</b></label>
          <input type="color" id="idSpoolColor" name="spoolColor" value="#ffffff" style="width:99%" class="w3-round-large w3-card" required>
        </div>
        <br>

        <label for="spoolWeightData"><b id="idSpoolWeightLbl">Spool Weight (g)</b></label>
        <input type="number" id="idSpoolWeightData" name="spoolWeightData" min="0" max="5000" step="0.1" class="w3-round-large w3-card" required>

        <label for="idFilamentTypeData"><b>Filament Type</b></label>
        <select class="w3-select w3-round-large w3-card" id="idFilamentTypeData" name="filamentTypeData" onchange="changeSpoolFilamentType()" required>
          <option value="0">ABS</option>
          <option value="1">ASA</option>
          <option value="2">Copper</option>
          <option value="3">HIPS</option>
          <option value="4">Nylon</option>
          <option value="5">PETG</option>
          <option value="6">PLA</option>
          <option value="7">PMMA</option>
          <option value="8">PolyC</option>
          <option value="9">PVA</option>
          <option value="10">TPE</option>
          <option value="11">TPU</option>
          <option value="12">User-1</option>
          <option value="13">User-2</option>
          <option value="14">User-3</option>
        </select>

        <label for="spoolDensityData"><b id="idFormSpoolDensityLbl"></b></label>
        <input type="number" id="idSpoolDensityData" name="spoolDensityData" min="0.01" max="5.0" step="0.01" class="w3-round-large w3-card" required>

        <label for="filamentDiaData"><b>Filament Diameter (mm)</b></label>
        <input type="number" id="idFilamentDiaData" name="filamentDiaData" min="1.0" max="4.0" step="0.01" class="w3-round-large w3-card" required>

        <button type="submit" style="width:48%;" class="w3-button w3-round-large w3-card w3-gray" onclick="putSpoolFormData()">Save</button>
        <button type="button" style="width:48%;" class="w3-button cancel w3-round-large w3-card w3-teal" onclick="unlockSpoolForm()">Done</button>
      </form>
    </div>


    <!-- FILAMENT DENSITY OPTIONS FORM -->
    <div class="form-popup" id="idDensityForm">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-theme-l2 w3-round-large w3-card">
        <h1>Filament Density</h1>

        <label for="idDensityTypeData"><b>Filament Type</b></label>
        <select class="w3-select w3-round-large w3-card" id="idDensityTypeData" name="densityTypeData" onchange="changeFilamentType()" required>
          <option value="0">ABS</option>
          <option value="1">ASA</option>
          <option value="2">Copper</option>
          <option value="3">HIPS</option>
          <option value="4">Nylon</option>
          <option value="5">PETG</option>
          <option value="6">PLA</option>
          <option value="7">PMMA</option>
          <option value="8">PolyC</option>
          <option value="9">PVA</option>
          <option value="10">TPE</option>
          <option value="11">TPU</option>
          <option value="12">User-1</option>
          <option value="13">User-2</option>
          <option value="14">User-3</option>
        </select>

        <label for="densityData"><b id="idFormDensityLbl"></b></label>
        <input type="number" id="idFilamentDensityData" name="densityData" style="width:65%" min="0.01" max="5.0" step="0.01" class="w3-round-large w3-card" required>

        <button type="submit" style="width:30%;" class="w3-button w3-round-large w3-card w3-gray" onclick="putDensityFormData()">Save</button>
        <button type="button" style="width:100%;" class="w3-button cancel w3-round-large w3-card w3-teal" onclick="unlockDensityForm()">Done</button>
      </form>
    </div>


    <!-- SAVE / RESTORE OPTIONS FORM -->
    <div class="form-popup" id="idSaveForm">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-theme-l2 w3-round-large w3-card">
        <h1>Save/Restore</h1>

        <div class="w3-col" style="width:48%;margin:10px 10px 0 0;"><b>Save Current Settings</b></div>
        <div class="w3-rest">
          <button id="sd1" type="button" style="width:90%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-gray" onclick="doSave()">Save</button>
        </div>

        <div class="w3-col" style="width:48%;margin:10px 10px 0 0;"><b>Restore Saved Settings</b></div>
        <div class="w3-rest">
          <button type="button" style="width:90%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-gray" onclick="doRestore()">Restore</button>
        </div>

        <div class="w3-col" style="width:48%;margin:23px 10px 0 0;"><b>Restart System</b></div>
        <div class="w3-rest">
          <button type="button" style="width:90%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-gray" onclick="doRestart()">Restart</button>
        </div>

        <div class="w3-col" style="width:48%;margin:23px 10px 0 0;"><b style="color:darkred">Reset Net</b></div>
        <div class="w3-rest">
          <button type="button" style="width:90%;margin:16px 0 0 0;background-color:darkred;color:white" class="w3-button w3-round-large w3-card" onclick="doResetNet()">Reset Net</button>
        </div>

        <div class="w3-col" style="width:48%;margin:23px 10px 0 0;"><b style="color:darkred">Factory Reset</b></div>
        <div class="w3-rest">
          <button type="button" style="width:90%;margin:16px 0 0 0;background-color:darkred;color:white" class="w3-button w3-round-large w3-card" onclick="doReset()">Reset Data</button>
        </div>

        <button type="button" style="width:100%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-teal" onclick="unlockSaveForm()">Done</button>
      </form>
    </div>


    <!-- WORKING POPUP -->
    <div id="idWorking" class="my-alert w3-panel w3-khaki w3-card-4 w3-round-large">
      <div class="my-alert-text w3-wide">
        <p>WORKING</p>
      </div>
    </div>


    <!-- RESET NET CONFIRMATION POPUP -->
    <div class="form-popup" id="idConfirmNetForm" style="z-index:10;">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-pale-red w3-round-large w3-card">
        <h2><b>Confirm Net Reset</b></h2><br>
        <p>!!! NET RESET will cause the loss of network credentials (SSID and password), network disconnection, and the scale to restart.</p><br>
        <p>Are you sure you want to RESET the NET?</p><br><br>
        <button type="button" style="width:48%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-red" onclick="confirmResetNet()">RESET</button>
        <button type="button" style="width:48%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-green" onclick="cancelResetNet()">CANCEL</button>
      </form>
    </div>

    <!-- RESET CONFIRMATION POPUP -->
    <div class="form-popup" id="idConfirmForm" style="z-index:10;">
      <form action="javascript:void(0)" class="form-container w3-card-4 w3-pale-red w3-round-large w3-card">
        <h2><b>Confirm Reset</b></h2><br>
        <p>!!! RESET will cause the loss of any settings that were previously stored.</p><br>
        <p>Are you sure you want to RESET?</p><br><br>
        <button type="button" style="width:48%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-red" onclick="confirmReset()">RESET</button>
        <button type="button" style="width:48%;margin:16px 0 0 0;" class="w3-button w3-round-large w3-card w3-green" onclick="cancelReset()">CANCEL</button>
      </form>
    </div>

  </body>


  <script>
    // Global variables.
    var msgInProcess;       // Used for serializing messsage requests.
    var popupActive;        // True if a popup is in process.
    var densityArray;       // Stores density data for each filament type.
    var spoolIdArray;       // Array of spool names.
    var spoolWeightArray;   // Array of spool weights.
    var spoolTypeArray;     // Array of filament types per spool.
    var spoolDensityArray;  // Array of filament density per spool.
    var spoolDiaArray;      // Array of filament diameters per spool.
    var colorArray;         // Array of filament colors per spool.
    var activeSpool;        // Spool being displayed in spool popup.
    var selectedSpoolIndex; // Index of selected spool (99 if none).
    var working;            // True if displaying "working" popup.
    var weightPrecision;    // Precision for weight fields.


    // Initialize globals on page load.
    window.onload = (event) => {
      msgInProcess = false;
      popupActive = false;
      working = false;
      weightPrecision = 1;
    }


    // Start the main page.  It will continue on its own.
    (function triggerMainPage() {
      if (!working) {
        loadDoc("/getMainPageData", updateMainPageData);
      }
      // Use setTimeout rather than setInterval since we don't want run
      // requests to queue up due to long executions of some links.
      setTimeout(triggerMainPage, 1000);
    })();


    // Add leading zero to single digit numnber.  Used for time values.
    function leadingZero(v) {
      if (v < 10) return "0" + v;
      return v;
    }


    // Format a number with commas every 3 decimal places.
    function formatNumber(num) {
      var num_parts = num.toString().split(".");
      num_parts[0] = num_parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
      return num_parts.join(".");
    }


    // Selected an option of a selection list based on its text value.
    function setSelectedIndex(s, v) {
      for (let i = 0; i < s.options.length; i++) {
        if (s.options[i].text == v) {
          s.options[i].selected = true;
          return;
        }
      }
    }


    // Returns the value of the selected item of a selection list.
    function getSelectedValue(s, v) {
      for (let i = 0; i < s.options.length; i++) {
        if (s.options[i].text == v) {
          return s.options[i].value;
        }
      }
    }


    // Unlock options so that the console may change them.
    function unlockOptions() {
      loadDoc("/unlockOptions", nullFunction);
    }


    // Null function.  Does nothing.  Used when no action is needed.
    function nullFunction() { }


    // Show the WORKING popup.
    function showWorking() {
      working = true;
      document.getElementById("idWorking").style.display = "block"
    }


    // Hide the WORKING popup.
    function clearWorking() {
      document.getElementById("idWorking").style.display = "none";
      working = false;
    }


    // Load a selected url followed by calling a specified function.
    function loadDoc(url, cFunction) {
      if (msgInProcess) {
        setTimeout(loadDoc, 25, url, cFunction);
      }
      else {
        msgInProcess = true;
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4) {
            if (this.status == 200) {
              cFunction(this);
            }
            else {
              //setTimeout(unlockOptions, 25);
            }
            msgInProcess = false;
          }
        };
        // xhttp.timeout = 2000;
        xhttp.open("GET", url, true);
        xhttp.send();
      }
    }


    // Send form data to the server.
    function putFormData(url, data, callback) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("POST", url, true);
      xhttp.setRequestHeader('Content-Type', 'application/json');
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            console.log(this.responseText);
          }
          callback(this);
          msgInProcess = false;
        }
      };
      xhttp.send(JSON.stringify(data));
    }


    // Update the main page display.
    function updateMainPageData(xhttp) {
      var json = JSON.parse(xhttp.responseText);
      var spoolSelected = json.SPOOL_SELECTED;

      // Weight units
      var weightUnits = json.WEIGHT_UNITS.trim();
      var weightLabel = " " + weightUnits;

      // WEB ID
      document.getElementById("idWebId").innerText = json.WEB_ID;


      // TEMPERATURE - "\xb0" represents the degrees symbol.
      var tempUnits = " \xb0" + json.TEMPERATURE_UNITS.trim();
      var value = parseFloat(json.TEMPERATURE);
      if (value != "-") {
        value = value.toFixed(json.TEMPERATURE_PRECISION) + tempUnits;
      }
      document.getElementById("idTemperature").innerText = value;

      // HUMIDITY
      value = parseFloat(json.HUMIDITY);
      if (value != "-") {
        value = value.toFixed(0)  + "%";
      }
      document.getElementById("idHumidity").innerText = value;

      // UP TIME
      var seconds = parseInt(json.UPTIME / 1000);
      var minutes = leadingZero(parseInt((seconds / 60) % 60));
      var hours = parseInt(seconds / 3600);
      seconds = leadingZero(seconds % 60);
      document.getElementById("idUptime").innerText = hours + ":" + minutes + ":" + seconds;

      // IP ADDRESS
      document.getElementById("idIpAddress").innerText = json.IP_ADDRESS;

      // SIGNAL STRENGTH
      value = json.SIGNAL_STRENGTH;
      var ss = document.getElementById("idSignalStrength").style;
      var signalPercent = Math.min(Math.max(2 * (value + 120), 0), 100);
      ss.width = signalPercent + "%";
      // Note: "\xa0" is equivalent to &nbsp.
      document.getElementById("idSignalStrength").innerText = value + "\xa0dBm";
      // document.getElementById("idSignalStrength").innerText = signalPercent + "%";

        if (value >= -79) {
          ss.backgroundColor = "lime";
          ss.color = "black";
        }
        else if (value >= -89) {
          ss.backgroundColor = "yellowgreen";
          ss.color = "black";
        }
        else if (value >= -99) {
          ss.backgroundColor = "yellow";
          ss.color = "black";
        }
        else if (value >= -109) {
          ss.backgroundColor = "orange";
          ss.color = "white";
        }
        else {
          ss.backgroundColor = "red";
          ss.color = "white";
        }

      if (spoolSelected) {
        // Adjust the scale data fieldset width.
        document.getElementById("idScaleDataFieldset").style.width = "100%";
        // Adjust the gross weight width.
        document.getElementById("idGrossWtContainer").style.width = "33%";

        // GROSS WEIGHT
        weightPrecision = json.WEIGHT_PRECISION;
        value = formatNumber(parseFloat(json.WEIGHT + json.SPOOL_WEIGHT).toFixed(weightPrecision));
        document.getElementById("idGrossWeight").innerText = value + weightLabel;

        // NET WEIGHT
        document.getElementById("idNetWeight").innerText =
          formatNumber(parseFloat(json.WEIGHT).toFixed(weightPrecision)) + weightLabel;

        // FILAMENT LENGTH
        var lengthUnits = json.LENGTH_UNITS.trim();
        document.getElementById("idLength").innerText =
          formatNumber(parseFloat(json.LENGTH).toFixed(json.LENGTH_PRECISION)) + " " + lengthUnits;

        // SPOOL ID
        var spoolId = json.SPOOL_NAME;
        document.getElementById("idSpoolId").innerText = spoolId;

        // SPOOL WEIGHT
        var spoolWeight = formatNumber(parseFloat(json.SPOOL_WEIGHT).toFixed(weightPrecision));
        document.getElementById("idSpoolWeight").innerText = spoolWeight + weightLabel;

        // FILAMENT TYPE
        var filamentType = json.FILEMANT_TYPE;
        document.getElementById("idFilamentType").innerText = filamentType;
        document.getElementById("idDensityLabel").innerText = filamentType + " Density";

        // FILAMENT DIAMETER
        var filamentDia = parseFloat(json.FILAMENT_DIAMETER).toFixed(2);
        value = filamentDia + " mm";
        document.getElementById("idFilamentDiameter").innerText = value;

        // FILAMENT DENSITY
        // Note the use of innerHTML in place of innerText to support superscript.
        var filamentDensity = parseFloat(json.FILAMENT_DENSITY).toFixed(2);
        value = filamentDensity + " g/cm" + "3".sup();
        document.getElementById("idDensity").innerHTML = value;

        // FILAMENT COLOR
        document.getElementById("idFilamentColor").style.backgroundColor = json.FILAMENT_COLOR;

        // Enable display of spool-related data.
        document.getElementById("idNetWtContainer").style.display = "block";
        document.getElementById("idLengthContainer").style.display = "block";
        document.getElementById("idSpoolContainer").style.display = "block";
      }
      else {
        // GROSS WEIGHT
        value = formatNumber(parseFloat(json.WEIGHT).toFixed(weightPrecision));
        document.getElementById("idGrossWeight").innerText = value + weightLabel;

        // Adjust the scale data fieldset width.
        document.getElementById("idScaleDataFieldset").style.width = "33%";

        // Adjust the gross weight width.
        document.getElementById("idGrossWtContainer").style.width = "100%";

        // Inhibit display of spool-related data since no spool is selected.
        document.getElementById("idNetWtContainer").style.display = "none";
        document.getElementById("idLengthContainer").style.display = "none";
        document.getElementById("idSpoolContainer").style.display = "none";
      }

      console.log(json);
    }


    /////////////////////////  FORMS ///////////////////////////


    ////////// Display form related functions. //////////
    function getDisplayFormData() {
      if (!popupActive) {
        loadDoc("/getDisplayFormData", openDisplayForm);
      }
      return false;
    }

    function openDisplayForm(xhttp) {
      var json = JSON.parse(xhttp.responseText);

      if (json.LOCKED == true) {
        popupActive = true;
        document.getElementById("idWeightUnitsData").value = json.WEIGHT_UNITS;
        document.getElementById("idLengthUnitsData").value = json.LENGTH_UNITS;
        document.getElementById("idTempUnitsData").value = json.TEMPERATURE_UNITS;
        document.getElementById("idBrightnessData").value = json.BRIGHTNESS;
        document.getElementById("idScrollDelayData").value = json.SCROLL_DELAY_S;

        var slider = document.getElementById("idBrightnessData");
        var sliderLabel = document.getElementById("idBrightnessLbl");
        sliderLabel.innerHTML = "Brightness: " + slider.value + "%";
        slider.oninput = function() {
          sliderLabel.innerHTML = "Brightness: " + this.value + "%";
        }

        var slider2 = document.getElementById("idScrollDelayData");
        var slider2Label = document.getElementById("idScrollDelayLbl");
        slider2.max = json.MAX_SCROLL_DELAY_S;
        slider2.step = json.SCROLL_DELAY_STEP_S;
        slider2Label.innerHTML = "Scroll Delay : " + slider2.value + " sec";
        slider2.oninput = function() {
          slider2Label.innerHTML = "Scroll Delay : " + this.value + " sec";
        }

        document.getElementById("idDisplayForm").style.display = "block";
      }
      else {
        alert("Cannot modify display data.  Options currently being modified.");
      }
    }

    function putDisplayFormData() {
      if (msgInProcess) {
        setTimeout(putDisplayFormData, 25);
      }
      else {
        msgInProcess = true;
        var wt = document.getElementById("idWeightUnitsData").value;
        var ln = document.getElementById("idLengthUnitsData").value;
        var t  = document.getElementById('idTempUnitsData').value;
        var b = document.getElementById("idBrightnessData").value;
        var s = document.getElementById("idScrollDelayData").value;
        var displayData = {
          weightUnitsData: wt,
          lengthUnitsData: ln,
          tempUnitsData: t,
          brightnessData: b,
          scrollDelayData: s
        };
        putFormData("/updateDisplayData", displayData, closeDisplayForm);
      }
      return false;
    }

    function unlockDisplayForm() {
      loadDoc("/unlockOptions", closeDisplayForm);
      return false;
    }

    function closeDisplayForm() {
      document.getElementById("idDisplayForm").style.display = "none";
      popupActive = false;
    }


    /////////// Scale form related functions. //////////
    function getScaleFormData() {
      if (!popupActive) {
        loadDoc("/getScaleFormData", openScaleForm);
      }
      return false;
    }

    function openScaleForm(xhttp) {
      var json = JSON.parse(xhttp.responseText);
      if (json.LOCKED == true) {
        popupActive = true;
        var weightPrecision = parseFloat(json.WEIGHT_PRECISION);
        var weightStep = 10 ** (-weightPrecision);
        weightStep = weightStep.toFixed(weightPrecision);
        var maxWeight = parseFloat(json.MAX_WEIGHT).toFixed(weightPrecision);
        var weightUnits = json.WEIGHT_UNITS.trim();
        var weight = parseFloat(json.CALIBRATE_WEIGHT).toFixed(weightPrecision);
        var avgSamples = json.AVG_SAMPLES;
        var avgSamplesMax = json.AVG_SAMPLES_MAX;
        var gain = json.LOAD_CELL_GAIN;

        document.getElementById("idScaleCalibrateWeightLbl").innerText =
          "Weight (" + weightUnits + ")";
        var wd = document.getElementById("idScaleCalibrateWeightData");
        wd.max = maxWeight;
        wd.step = weightStep;
        wd.value = weight;
        document.getElementById("idAverageSamples").value = avgSamples;
        document.getElementById("idAverageSamples").max = avgSamplesMax;
        document.getElementById("idScaleGainData").value = gain;
        document.getElementById("idScaleForm").style.display = "block";
      }
      else {
        alert("Cannot modify scale data.  Options currently being modified.");
      }
    }

    function putScaleFormData() {
      if (msgInProcess) {
        setTimeout(putScaleFormData, 25);
      }
      else {
        msgInProcess = true;
        var avgElement = document.getElementById("idAverageSamples");
        var wtElement  = document.getElementById("idScaleCalibrateWeightData");
        if (!avgElement.checkValidity() || !wtElement.checkValidity()) {
          msgInProcess = false;
          return false;
        }
        var avg  = avgElement.value;
        var wt   = wtElement.value;
        var gain = document.getElementById("idScaleGainData").value;

        var scaleData = {
          calWeightData: wt,
          avgSamples:    avg,
          scaleGain:     gain
        };

        putFormData("/updateScaleData", scaleData, closeScaleForm);
      }

      return false;
    }

    function unlockScaleForm() {
      loadDoc("/unlockOptions", closeScaleForm);
      return false;
    }

    function closeScaleForm() {
      document.getElementById("idScaleForm").style.display = "none";
      popupActive = false;
    }

    function doTare() {
      if (!popupActive) {
        showWorking();
        loadDoc("/doTare", handleTareResult);
      }
      return false;
    }

    function doOptionsTare() {
      showWorking();
      loadDoc("/doTare", handleTareResult);
      return false;
    }

    function handleTareResult(xhttp) {
      clearWorking();
      var json = JSON.parse(xhttp.responseText);
      if (json.TARE_RESULT == false) {
        alert("Tare failed.");
      }
    }

    function doScaleCalibrate() {
      if (msgInProcess) {
        setTimeout(doScaleCalibrate, 25);
      }
      else {
        msgInProcess = true;
        var wtElement = document.getElementById("idScaleCalibrateWeightData");
        if (!wtElement.checkValidity()) {
          wtElement.reportValidity();
          msgInProcess = false;
          return false;
        }
        showWorking();
        var wt = wtElement.value;
        var calData = { calWeightData: wt };

        putFormData("/doScaleCalibrate", calData, handleScaleCalResult);
      }

      return false;
    }

    function handleScaleCalResult(xhttp) {
      clearWorking();
      if (xhttp.status == 200) {
        var json = JSON.parse(xhttp.responseText);
        if (json.CAL_RESULT == false) {
          alert("Calibration failed.");
        }
      }
    }

    ////////// Spool form related functions. //////////
    function getSpoolFormData() {
      if (!popupActive) {
        loadDoc("/getSpoolFormData", openSpoolForm);
      }
      return false;
    }

    function openSpoolForm(xhttp) {
      var json = JSON.parse(xhttp.responseText);
      if (json.LOCKED == true) {
        popupActive = true;

        startSpool = json.START_SPOOL;
        activeSpool = startSpool;
        var spoolIsSelected = json.SPOOL_SELECTED;
        selectedSpoolIndex = spoolIsSelected ? startSpool : 99;

        spoolIdArray = json.SPOOL_NAMES;
        spoolWeightArray = json.SPOOL_WEIGHTS;
        spoolTypeArray = json.FILAMENT_TYPES;
        spoolDensityArray = json.SPOOL_DENSITY;
        spoolDiaArray = json.FILAMENT_DIAMETERS;
        densityArray = json.DENSITY;
        colorArray = json.COLORS;

        weightPrecision = parseFloat(json.WEIGHT_PRECISION);
        var weightStep = 10 ** (-weightPrecision);
        weightStep = weightStep.toFixed(weightPrecision);
        var maxWeight = parseFloat(json.MAX_WEIGHT).toFixed(weightPrecision);
        var weightUnits = json.WEIGHT_UNITS.trim();

        document.getElementById("idSpoolDensityData").max = json.MAX_DENSITY;
        document.getElementById("idSpoolDensityData").min = json.MIN_DENSITY;
        document.getElementById("idFormSpoolDensityLbl").innerHTML = "Filament Density (g/cm" + "3".sup() + ")";

        document.getElementById("idSpoolIdData").maxLength = json.MAX_NAME_LEN;
        document.getElementById("idSpoolWeightLbl").innerText = "Spool Weight (" + weightUnits + ")";
        document.getElementById("idSpoolWeightData").max = maxWeight;
        document.getElementById("idSpoolWeightData").step = weightStep;
        document.getElementById("idSpoolForm").style.display = "block";

        manageSpoolButtons();
        selectSpool();

      }
      else {
        alert("Cannot modify the spool data.  Options currently being modified.");
      }
    }

    function putSpoolFormData() {
      if (msgInProcess) {
        setTimeout(putSpoolFormData, 25);
      }
      else {
        msgInProcess = true;
        var wtElement = document.getElementById("idSpoolWeightData");
        var diElement = document.getElementById("idFilamentDiaData");
        var idElement = document.getElementById("idSpoolIdData");
        var deElement = document.getElementById("idSpoolDensityData");
        if (!wtElement.checkValidity() || !diElement.checkValidity() ||
            !deElement.checkValidity() || !idElement.checkValidity()) {
          msgInProcess = false;
          return false;
        }

        showWorking();

        var wt = wtElement.value;
        var di = diElement.value;
        var id = idElement.value;
        var de = deElement.value;
        var co = document.getElementById("idSpoolColor").value;


        var selected = document.getElementById("idSpoolSelect").checked;
        var ty = document.getElementById('idFilamentTypeData').value;
        var spoolData = {
          spoolIndex: activeSpool,
          spoolSelected: selected,
          spoolIdData: id,
          spoolWeightData: wt,
          filamentTypeData: ty,
          spoolDensity: de,
          filamentDiaData: di,
          colorData: co
        };
        if (selected) {
          selectedSpoolIndex = activeSpool;
        }
        else if (activeSpool == selectedSpoolIndex) {
          selectedSpoolIndex = 99;
        }

        spoolIdArray[activeSpool] = id;
        spoolWeightArray[activeSpool] = wt;
        spoolTypeArray[activeSpool] = ty;
        spoolDiaArray[activeSpool] = di;
        spoolDensityArray[activeSpool] = de;
        colorArray[activeSpool] = co;

        putFormData("/updateSpoolData", spoolData, finishSpoolFormPut);
      }

      return false;
    }

    function finishSpoolFormPut(xhttp) {
      clearWorking();
    }

    function incrementSpool() {
      if (activeSpool < spoolIdArray.length - 1) {
        activeSpool++;
        manageSpoolButtons();
        selectSpool();
      }
      return false;
    }

    function decrementSpool() {
      if (activeSpool > 0) {
        activeSpool--;
        manageSpoolButtons();
        selectSpool();
      }
      return false;
    }

    function changeSpoolFilamentType() {
      let filamentType = document.getElementById('idFilamentTypeData').value;
      document.getElementById("idSpoolDensityData").value = parseFloat(densityArray[filamentType]).toFixed(2);
    }

    function manageSpoolButtons() {
      document.getElementById("incSpool").disabled = (activeSpool >= spoolIdArray.length - 1);
      document.getElementById("decSpool").disabled = (activeSpool <= 0);
    }

    function selectSpool() {
      document.getElementById("idSpoolSelect").checked = (selectedSpoolIndex == activeSpool);
      document.getElementById("idSpoolIdData").value = spoolIdArray[activeSpool];
      document.getElementById("idSpoolWeightData").value = formatNumber(parseFloat(spoolWeightArray[activeSpool]).toFixed(weightPrecision));
      document.getElementById('idFilamentTypeData').value = spoolTypeArray[activeSpool];
      document.getElementById("idSpoolDensityData").value = spoolDensityArray[activeSpool];
      document.getElementById("idFilamentDiaData").value = spoolDiaArray[activeSpool];
      document.getElementById("idSpoolColor").value = colorArray[activeSpool];
    }

    function unlockSpoolForm() {
      loadDoc("/unlockOptions", closeSpoolForm);
      return false;
    }

    function closeSpoolForm() {
      document.getElementById("idSpoolForm").style.display = "none";
      popupActive = false;
    }


    ////////// Filament form related functions. //////////
    // Send a lock density form request to the server.  If successful, will lock
    // the server so that local ooption setting is disabled.  The server will
    // respond with the openDensityForm data.
    function getDensityFormData() {
      if (!popupActive) {
        loadDoc("/getDensityFormData", openDensityForm);
      }
      return false;
    }

    // Receive density form info from the server, and make the density form visible.
    function openDensityForm(xhttp) {
      var json = JSON.parse(xhttp.responseText);

      // We successfully got the lock from the server, so go ahead and show the form.
      if (json.LOCKED == true) {
        popupActive = true;

        var filamentType = json.FILEMANT_TYPE;
        densityArray = json.DENSITY;

        document.getElementById('idDensityTypeData').value = filamentType;
        let label = document.getElementById('idDensityTypeData').options[filamentType].text;
        document.getElementById("idFormDensityLbl").innerHTML = label + " Density (g/cm" + "3".sup() + ")";

        document.getElementById("idFilamentDensityData").max = json.MAX_DENSITY;
        document.getElementById("idFilamentDensityData").min = json.MIN_DENSITY;
        document.getElementById("idFilamentDensityData").value = parseFloat(densityArray[filamentType]).toFixed(2);

        document.getElementById("idDensityForm").style.display = "block";
      }
      else {
        alert("Cannot modify the density.  Options currently being modified.");
      }
    }

    function putDensityFormData() {
      if (msgInProcess) {
        setTimeout(putDensityFormData, 25);
      }
      else {
        msgInProcess = true;
        var dElement = document.getElementById("idFilamentDensityData");
        if (!dElement.checkValidity()) {
          msgInProcess = false;
          return false;
        }
        showWorking();
        var d = dElement.value;
        var t = document.getElementById("idDensityTypeData").value;
        densityArray[t] = d;
        var density = {
          densityData: d,
          filamentTypeData: t
        };

        putFormData("/updateDensityData", density, clearWorking);
      }

      return false;
    }

    function changeFilamentType() {
      let filamentType = document.getElementById('idDensityTypeData').value;
      let label = document.getElementById('idDensityTypeData').options[filamentType].text;
      document.getElementById("idFormDensityLbl").innerHTML = label + " Density (g/cm" + "3".sup() + ")";
      document.getElementById("idFilamentDensityData").value = parseFloat(densityArray[filamentType]).toFixed(2);
    }

    // Send an unlock message to the server and hide the density form.
    function unlockDensityForm() {
      loadDoc("/unlockOptions", closeDensityForm);
      return false;
    }

    // Make the density form disappear.
    function closeDensityForm() {
      document.getElementById("idDensityForm").style.display = "none";
      popupActive = false;
    }


    ////////// Save/Restore form related functions. //////////
    function startSaveForm() {
      if (!popupActive) {
        loadDoc("/lockOptions", openSaveForm);
      }
      return false;
    }

    function openSaveForm(xhttp) {
      var json = JSON.parse(xhttp.responseText);
      if (json.LOCKED == true) {
        popupActive = true;
        document.getElementById("idSaveForm").style.display = "block";
      }
      else {
        alert("Cannot modify data.  Options currently being modified.");
      }
    }

    function doSave() {
      showWorking();
      loadDoc("/doSave", checkSaveStatus);
      return false;
    }

    function checkSaveStatus(xhttp)
    {
      clearWorking();
      var json = JSON.parse(xhttp.responseText);
      if (json.SAVE_RESULT == true) {
        closeSaveForm();
        doReloadPage();
      }
      else {
        alert("Save failed for unknown reason.");
      }
    }

    function doRestore() {
      showWorking();
      loadDoc("/doRestore", checkRestoreStatus);
      return false;
    }

    function checkRestoreStatus(xhttp)
    {
      clearWorking();
      var json = JSON.parse(xhttp.responseText);
      if (json.RESTORE_RESULT == true) {
        closeSaveForm();
        doReloadPage();
      }
      else {
        alert("Restore failed for unknown reason.");
      }
    }

    function doRestart() {
      loadDoc("/doRestart", reloadPage);
      return false;
    }

    function doResetNet() {
      document.getElementById("idConfirmNetForm").style.display = "block";
      return false;
    }

    function confirmResetNet() {
      loadDoc("/doResetNet", reloadPage);
      return false;
    }

    function cancelResetNet() {
      document.getElementById("idConfirmNetForm").style.display = "none";
      return false;
    }

    function doReset() {
      document.getElementById("idConfirmForm").style.display = "block";
      return false;
    }

    function confirmReset() {
      loadDoc("/doReset", reloadPage);
      return false;
    }

    function cancelReset() {
      document.getElementById("idConfirmForm").style.display = "none";
      return false;
    }

    function reloadPage() {
        setTimeout(doReloadPage, 7000);
    }

    function doReloadPage() {
      location.reload();
    }

    function unlockSaveForm() {
      loadDoc("/unlockOptions", closeSaveForm);
      return false;
    }

    function closeSaveForm() {
      document.getElementById("idSaveForm").style.display = "none";
      popupActive = false;
    }

  </script>

</html>
)=====";  // End gRootPage[].

#endif // WEBPAGES_H