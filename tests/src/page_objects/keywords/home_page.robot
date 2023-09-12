*** Settings ***
Library  SeleniumLibrary
Library  ../libraries/custom.py
Library    OperatingSystem
Library    String
Variables  ../locators/locators.py

*** Keywords ***
Click Add Files Button
    Click Element  ${AddFilesButton}
    Log  Click Add Files Button  console=yes

Click Extract And Save Button
    [Arguments]    ${timeout}
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    Click Element  ${ExtractAndSaveButton}
    Wait Until Element Contains    ${ProgressBar}    100%    timeout=${timeout}
    Sleep    10s
    Log  Click Extract And Save Button  console=yes

Click Remove Button
    Click Element  ${RemoveButton}
    Log  Click Remove Button  console=yes

Click Start Button
    Click Element  ${StartButton}
    Log  Click Start Button  console=yes
    Wait Until Element Is Enabled    ${StartButton}
    
Log Console Is Visible
    Wait Until Element Is Visible   ${CollapseLogsButton}
    ${variable}    Run Keyword And Return Status    Element Should Be Visible   ${LogsTitle}
    RETURN    ${variable}

 Click Show/Hide Logs Button
    Click Element    ${CollapseLogsButton}

Unhide Log Window
    Log    Unhide window with logs
    ${status} =   Log Console Is Visible
    Run Keyword If    ${status} == False  Click Show/Hide Logs Button
    Wait Until Element Is Visible   xpath=${LogsTitle}

Check If Log Console Contains
    [Arguments]    ${message}    ${timeout}=5
    Unhide Log Window
    Wait Until Element Contains    ${LogsConsole}    ${message}    ${timeout}
    
Check If Log Console Does Not Contain Errors
    Log    Check if there are no errors or warnings in the Log Console   console=yes
    ${logs}=    Get Text    ${LogsConsole}
    @{log_list}=    Split To Lines    ${logs}
    FOR    ${line}    IN    @{log_list}
        Should Not Contain    ${line}    ${space}error${space}    ignore_case=True
    END
 
Validate File Details In Log Console
    [Arguments]    ${file}
    Log    Validate file details in Log Console    console=yes
    Check If Log Console Contains    Total size: ${file}[archive_size_bytes] bytes    10
    Check If Log Console Contains    Done. Creating ZIP file
    
Validate Output Description
    [Arguments]    ${expected_output}
    Log    Validate Output Description    console=yes
    ${output} =    Get Text    ${Description}  
    Element Should Contain    ${Description}    ${expected_output}
    ...    error=Description validation failed. Actual: ${output}, expected: ${expected_output}

Validate Output Archive Files Number
    [Arguments]    ${files_num}
    Log    Validate archive files number    console=yes
    ${output_filenum} =    Get Text    ${FileNum} 
    Element Should Contain    ${FileNum}    ${files_num}
    ...    error=Validation files number failed. Actual: ${output_filenum}, expected: ${files_num}

Validate Output Archive File Size
    [Arguments]    ${size}
    Log    Validate archive size    console=yes
    ${output_size} =    Get Text    ${FileSize}
    ${size} =   Convert To Mega    ${size} 
    Element Should Contain    ${FileSize}    ${size}
    ...    error=Validation file size failed. Actual: ${output_size}, expected: ${size}
