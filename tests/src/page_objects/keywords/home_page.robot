*** Settings ***
Library  SeleniumLibrary
Variables  ../locators/locators.py

*** Keywords ***
Click Add Files Button
    Click Element  ${AddFilesButton}
    Log  Click Add Files Button  console=yes

Click Extract And Save Button
    Click Element  ${ExtractAndSaveButton}
    Log  Click Extract And Save Button  console=yes

Click Remove Button
    Click Element  ${RemoveButton}
    Log  Click Remove Button  console=yes

Click Start Button
    Click Element  ${StartButton}
    Log  Click Start Button  console=yes

Log Console Is Visible
    Wait Until Element Is Visible   xpath=${CollapseLogsButton}
    Run Keyword And Return Status    Element Should Be Visible   xpath=${LogsTitle}
    ${variable}=    Run Keyword And Return Status    Element Should Be Visible   xpath=${LogsTitle}
    RETURN    ${variable}

Open Log Console
    Click Element    xpath=${CollapseLogsButton}

Unhide Log Window
    ${status} =   Log Console Is Visible
    Run Keyword If    ${status} == False  Open Log Console
    Wait Until Element Is Visible   xpath=${LogsTitle}

Check If Log Console Contains
    [Arguments]    ${message}    ${timeout}=5
    Unhide Log Window    
    Wait Until Element Contains    ${LogsConsole}    ${message}    ${timeout}
    Element Should Not Contain    ${LogsConsole}    error
