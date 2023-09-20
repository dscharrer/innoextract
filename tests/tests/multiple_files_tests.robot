*** Settings ***
Documentation  Multiple Files Tests
Library        OperatingSystem
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/test_files/test_files.resource
Resource       ../src/page_objects/keywords/ubuntu.robot
Library        ../src/page_objects/libraries/browser_lib.py
Library        Collections
Variables      ../src/page_objects/locators/locators.py
Variables      ../src/test_files/test_files.yaml
Suite Setup   Prepare Test Environment
Test Setup    Prepare For Test

*** Variables ***
${BROWSER}             Firefox
${HOME_PAGE_PATH}      http://localhost:8000
${TEST_FILE}           ${file_4mb}
${MULTI_PART_TEST_FILE}           ${multi_part_4mb}
${EXTRACTION_TIMEOUT}    30s
${INPUT_TEST_FILES_PATH}        ${CURDIR}${/}../src/test_files/

*** Test Cases ***
# TODO: Test is failing because of known issue
Extract two files one by one test
    [documentation]  Extract two files one by one
    [tags]  multiple  performance

    # Adding and Extracting First File
    
    Extract File  ${TEST_FILE}    ${TEST_FILE}[path]
    Check If Zip File Is Not Empty   ${DOWNLOAD_PATH}    ${TEST_FILE}
    # Adding and Extracting Second File
    Rename Downloaded Zip File Name  ${DOWNLOAD_PATH}    ${TEST_FILE}
    Clean Input List
    Extract File  ${TEST_FILE}    ${TEST_FILE}[path]
    Check If Zip File Is Not Empty   ${DOWNLOAD_PATH}    ${TEST_FILE}


Extract multiple files test
    [documentation]  Extract file consisting of multiple files
    [tags]  multiple  performance
    Log To Console    Extracting file consisting of multiple files
    Extract Multiple Files    ${MULTI_PART_TEST_FILE}    ${DOWNLOAD_PATH}
    Check If Zip File Is Not Empty   ${DOWNLOAD_PATH}    ${MULTI_PART_TEST_FILE}
    Clean Input List

Extract all files test
    [documentation]  Extract all test files
    [tags]  multiple  performance    all
    FOR    ${file}    IN    @{TestFiles}
        Log To Console    Extracting ${file}[name]
         ${downloaded_file_path}  Set Variable  ${DOWNLOAD_PATH}${TEST_FILE}[archive_name].zip
        ${path}    Set Variable   ${INPUT_TEST_FILES_PATH}${file}[name]
        Extract File    ${file}    ${path}    ${file}[extraction_time]
        Wait Until Created  ${downloaded_file_path}
    END
    Close Browser

*** Keywords ***
Extract File
    [Arguments]  ${test_file}    ${test_file_path}    ${EXTRACTION_TIMEOUT}=30s
    Click Add Files Button
    Ubuntu Upload Test File  ${test_file_path}
    Click Start Button
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}

Extract Multiple Files
    [Arguments]  ${test_file}  ${DOWNLOAD_PATH}
    @{file_list}    Create List    
    FOR    ${i}    IN RANGE    1    ${test_file}[parts]
        ${file}    Catenate    ${test_file}[name]-${i}.bin
        Append To List   ${file_list}    ${file}
    END
    Append To List   ${file_list}    ${test_file}[name].exe
    Log To Console    ${file_list}

    FOR    ${file}    IN    @{file_list}
        ${test_file_path}    Catenate    SEPARATOR=    ${test_file}[path]/${file}
        Click Add Files Button
        Ubuntu Upload Test File  ${test_file_path}
    END
    Click Start Button
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    
Clean Input List
    ${radio_buttons_amount}    Get Element Count    ${RadioButton}
    FOR    ${button}    IN RANGE    1    ${radio_buttons_amount+1}
        Log To Console    Found ${radio_buttons_amount} radio buttons
        Log    Removing entry from Input List ${button}    console=yes
        Click Button    ${RadioButton}
        Click Remove Button       
    END
    Page Should Not Contain Button    ${RadioButton}

