*** Settings ***
Documentation  Multiple Files Tests
Library        OperatingSystem
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/page_objects/keywords/windows.robot
Resource       ../src/test_files/test_files.resource
Library        ../src/page_objects/libraries/browser_lib.py
Library    Collections
Variables      ../src/page_objects/locators/locators.py

*** Variables ***
${BROWSER}             Firefox
${HOME_PAGE_PATH}      http://127.0.0.1:8000/index.html
${TEST_FILE}           ${file_4mb}
${MULTI_PART_TEST_FILE}           ${multi_part_4mb}

*** Test Cases ***
# TODO: Test is failing because of known issue
Extract two files one by one test
    [documentation]  Extract two files one by one
    [tags]  multiple  performance

    ${download_path}  Create Unique Download Path
    ${profile}  create_profile  ${download_path}
    Opening Browser  ${HOME_PAGE_PATH}  ${browser}  ${profile}

    # Adding and Extracting First File
    Extract File  ${TEST_FILE}  ${download_path}
    Check If Zip File Is Not Empty   ${download_path}
    # Adding and Extracting Second File
    Rename Downloaded Zip File Name  path=${download_path}
    Clean Input List
    Extract File  ${TEST_FILE}  ${download_path}
    Check If Zip File Is Not Empty   ${download_path}
    Close Browser

Extract multiple files test
    [documentation]  Extract file consisting of multiple files
    [tags]  multiple  performance

    ${download_path}  Create Unique Download Path
    ${profile}  create_profile  ${download_path}
    Opening Browser  ${HOME_PAGE_PATH}  ${browser}  ${profile}
    Log To Console    Extracting file consisting of multiple files
    Extract Multiple Files    ${MULTI_PART_TEST_FILE}    ${download_path}
    Check If Zip File Is Not Empty   ${download_path}
    Clean Input List
    Close Browser

*** Keywords ***
Extract File
    [Arguments]  ${test_file}  ${download_path}
    Click Add Files Button
    Upload Test File  ${test_file}[path]
    Click Start Button
    Click Extract And Save Button

Extract Multiple Files
    [Arguments]  ${test_file}  ${download_path}
    @{file_list}    Create List    
    FOR    ${i}    IN RANGE    1    ${test_file}[parts]
        ${file}    Catenate    ${test_file}[name]-${i}.bin
        Append To List   ${file_list}    ${file}
    END
    Append To List   ${file_list}    ${test_file}[name].exe
    Log To Console    ${file_list}

    FOR    ${file}    IN    @{file_list}
        ${test_file_path}    Catenate    SEPARATOR=    ${test_file}[path]\\${file}
        Click Add Files Button
        Upload Test File  ${test_file_path}
    END
    Click Start Button
    Click Extract And Save Button
    
Clean Input List
    ${radio_buttons_amount}    Get Element Count    ${RadioButton}
    FOR    ${button}    IN RANGE    1    ${radio_buttons_amount+1}
        Log To Console    Found ${radio_buttons_amount} radio buttons
        Log    Removing entry from Input List ${button}    console=yes
        Click Button    ${RadioButton}
        Click Remove Button       
    END
    Page Should Not Contain Button    ${RadioButton}
