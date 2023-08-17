*** Settings ***
Library    OperatingSystem
Library    SeleniumLibrary
Variables  ../locators/locators.py

*** Keywords ***
Ubuntu Upload Test File
    [Arguments]  ${file_path}
    OperatingSystem.Run    xdotool key ctrl+l
    Sleep    2s
    OperatingSystem.Run    xdotool type ${file_path}
    Sleep    2s
    OperatingSystem.Run    xdotool --window %1
    OperatingSystem.Run    xdotool key KP_Enter
    OperatingSystem.Run    xdotool key KP_Enter
    OperatingSystem.Run    xdotool --window %1
    Wait Until Element Is Enabled   ${StartButton}
