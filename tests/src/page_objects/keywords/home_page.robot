*** Settings ***
Library  SeleniumLibrary
Library  ../libraries/custom.py
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

Validate Output Description
    [Arguments]    ${expected_output}
    ${output} =    Get Text    ${Description}  
    Wait Until Element Contains    ${Description}    ${expected_output}
    ...    error=Expected output: ${expected_output}, read from Web: ${output}

Validate Archive File
    [Arguments]    ${size}    ${files_num}
    ${output_filenum} =    Get Text    ${FileNum} 
    Wait Until Element Contains    ${FileNum}    ${files_num}
    ...    error=Number of files: ${output_filenum} expected: ${files_num}

    ${output_size} =    Get Text    ${FileSize}
    ${size} =   Convert To Mega    ${size} 
    Wait Until Element Contains    ${FileSize}    ${size}    error=File size: ${output_size}, expected: ${size}
