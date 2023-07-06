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

*** Variables ***
${BROWSER}             Firefox
${HOME_PAGE_PATH}      http://127.0.0.1:8000/index.html
${TEST_FILE}           ${file_4mb}

*** Test Cases ***
Extract two files one by one test
    [documentation]  Extract two files one by one
    [tags]  multiple  performance

    ${download_path}  Create Unique Download Path
    ${profile}  create_profile  ${download_path}
    Opening Browser  ${HOME_PAGE_PATH}  ${browser}  ${profile}

    # Adding and Extracting First File
    Extract File  ${TEST_FILE}  ${download_path}

    # Adding and Extracting Second File
    Rename Downloaded Zip File Name  path=${download_path}
    Extract File  ${TEST_FILE}  ${download_path}

    Close Browser

Extract two files at once test
    [documentation]  Extract two files one by one
    [tags]  multiple  performance

Extract multiple files at once test
    [documentation]  Extract two files one by one
    [tags]  multiple  performance
    
*** Keywords ***
Extract File
    [Arguments]  ${test_file}  ${download_path}
    Click Add Files Button
    Upload Test File  ${test_file}[path]
    Click Start Button
    Click Extract And Save Button
    Validate Zip File   ${download_path}

Extract Multiple Files
    [Arguments]  @{test_files}  ${download_path}
    FOR    ${test_file}    IN    @{test_files}
        Click Add Files Button
        Upload Test File  ${test_file}[path]
    END
    Click Start Button
    Click Extract And Save Button
    Validate Zip File   ${download_path}