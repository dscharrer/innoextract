*** Settings ***
Documentation  Smoke Tests
Library        OperatingSystem
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/page_objects/keywords/ubuntu.robot
Resource       ../src/test_files/test_files.resource
Library        ../src/page_objects/libraries/browser_lib.py
Suite Setup   Prepare Test Environment
Test Setup    Prepare For Test
Test Teardown    Clean After Test

*** Variables ***
${TEST_FILE}           ${file_4mb}
${EXTRACTION_TIMEOUT}    60s


*** Test Cases ***
Extract test file
    [documentation]  Extract smoke file successfully
    [tags]  Smoke
    #TODO: Move some steps to TEST SETUP or SUITE SETUP
    ${downloaded_file_path}  Set Variable  ${DOWNLOAD_PATH}${TEST_FILE}[archive_name].zip
    
    Click Add Files Button
    Ubuntu Upload Test File  ${TEST_FILE}[path]
    Click Start Button
    Check If Log Console Contains    Opening "${TEST_FILE}[name]"
    Validate Output Description  ${TEST_FILE}[archive_name]
    Validate Output Archive File Size  ${TEST_FILE}[archive_size_bytes]
    Validate Output Archive Files Number    ${TEST_FILE}[files_in_archive]
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    Wait Until Created  ${downloaded_file_path}

    Validate and Unzip Test File    ${downloaded_file_path}
    Validate File Details In Log Console    ${TEST_FILE}
    Check If Log Console Does Not Contain Errors
    Check If JS Console Does Not Contain Errors