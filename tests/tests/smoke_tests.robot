*** Settings ***
Documentation  Smoke Tests
Library        OperatingSystem
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/page_objects/keywords/windows.robot
Resource       ../src/test_files/test_files.resource
Library        ../src/page_objects/libraries/browser_lib.py

*** Variables ***
${BROWSER}             Firefox
${DOWNLOAD_FILE_NAME}  innoout.zip
${HOME_PAGE_PATH}      http://127.0.0.1:8000/index.html
${TEST_FILE}           ${10k_files}

*** Test Cases ***
Extract test file
    [documentation]  Extract smoke file successfully
    [tags]  Smoke
    ${download_path}  Create Unique Download Path
    ${profile}  create_profile  ${download_path}
    Opening Browser  ${HOME_PAGE_PATH}  ${browser}  ${profile}
    Click Add Files Button
    Upload Test File  ${TEST_FILE}[path]
    Click Start Button
    Validate Output Description  ${TEST_FILE}[output]
    Validate Archive File  ${TEST_FILE}[archive_size_bytes]  ${TEST_FILE}[files_in_archive]
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    Click Extract And Save Button
    ${downloaded_file_path}  Set Variable  ${download_path}${DOWNLOAD_FILE_NAME}
    Log  Validate file created: ${downloaded_file_path}  console=yes
    Wait Until Created  ${downloaded_file_path}
    Sleep  1s
    Log  Validate file is not empty: ${downloaded_file_path}  console=yes
    File Should Not Be Empty  ${downloaded_file_path}
    Close Browser
    