*** Settings ***
Documentation  Smoke Tests
Library        OperatingSystem
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/page_objects/keywords/ubuntu.robot
Resource       ../src/test_files/test_files.resource
Resource       /tmp/TestArtifacts/large_test_files.resource
Library        ../src/page_objects/libraries/browser_lib.py
Suite Setup   Prepare Test Environment
Test Setup    Prepare For Test
Test Teardown    Clean After Test

*** Variables ***
${TEST_FILE}           ${file_1}
${EXTRACTION_TIMEOUT}    ${file_1}[extraction_time]
${PATH_TO_PATTERN_FOLDER}    ${file_1}[pattern_path]  

*** Test Cases ***
Extract and validate a large test file
    [documentation]  Extract and validate a large test file
    [tags]    Daily    Regression    Large
    ${downloaded_file_path}  Set Variable  ${DOWNLOAD_PATH}${TEST_FILE}[archive_name].zip
    # Add a file for extraxtion
    Click Add Files Button
    Ubuntu Upload Test File    ${large_files_upload_path}${TEST_FILE}[name]
    # Load the file
    Click Load Button
    Check If Log Console Contains    Opening "${TEST_FILE}[name]"
    Validate Output Description  ${TEST_FILE}[archive_name]
    Validate Output Archive File Size  ${TEST_FILE}[archive_size_bytes]
    Validate Output Archive Files Number    ${TEST_FILE}[files_in_archive]
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    # Extract and create a ZIP file
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    Wait Until Created  ${downloaded_file_path}
    # Rename folder name
    ${downloaded_file_path_new_name}=    Remove Spaces From File Name    ${downloaded_file_path}
    # Unzip extracted ZIP file
    Validate and Unzip Test File    ${downloaded_file_path_new_name}
    # Validate output
    Check If Log Console Contains    "status":"Completed successfully"
    Check If Log Console Does Not Contain Errors
    Check If JS Console Does Not Contain Errors

Extract and compare directory and files tree
    [documentation]  Extract and compare directory and files tree
    [tags]    Daily    Regression    Large
    # TODO: This test fails because of bug. One empty folder and 1one empty file are not in ZIP folder.
    # Either pattern folder will be adjusted or bug will be fixed
    ${downloaded_file_path}  Set Variable  ${DOWNLOAD_PATH}${TEST_FILE}[archive_name].zip
    # Add a file for extraxtion 
    Click Add Files Button
    Ubuntu Upload Test File    ${large_files_upload_path}${TEST_FILE}[name]
    # Load the file
    Click Load Button
    Check If Log Console Contains    Opening "${TEST_FILE}[name]"
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    # Extract and create a ZIP file
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    Wait Until Created  ${downloaded_file_path}
    # Rename folder name
    ${downloaded_file_path_new_name}=    Remove Spaces From File Name    ${downloaded_file_path}
    Unzip File    ${downloaded_file_path_new_name}
    ${unzipped_new_folder_name}=     Remove Spaces From Directory Name   ${DOWNLOAD_PATH}${TEST_FILE}[archive_name]
    # Validate output
    Compare Directory And Files Tree    ${unzipped_new_folder_name}    ${PATH_TO_PATTERN_FOLDER}

Extract and compare files checksum
    [documentation]  Extract and compare files checksums and sizes
    [tags]    Daily    Regression    Large
    # TODO: This test fails because of bug. One empty folder and 1one empty file are not in ZIP folder.
    # Either pattern folder will be adjusted or bug will be fixed
    ${downloaded_file_path}  Set Variable  ${DOWNLOAD_PATH}${TEST_FILE}[archive_name].zip
    # Add a file for extraxtion 
    Click Add Files Button
    Ubuntu Upload Test File    ${large_files_upload_path}${TEST_FILE}[name]
    # Load the file
    Click Load Button
    Check If Log Console Contains    Opening "${TEST_FILE}[name]"
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    # Extract and create a ZIP file
    Click Extract And Save Button    ${EXTRACTION_TIMEOUT}
    Wait Until Created  ${downloaded_file_path}
    # Rename folder name
    ${downloaded_file_path_new_name}    Replace String    ${downloaded_file_path}    ${space}    ${empty}
    Copy File    ${downloaded_file_path}    ${downloaded_file_path_new_name}
    Wait Until Created    ${downloaded_file_path_new_name}
    # Unzip extracted ZIP file
    Validate and Unzip Test File    ${downloaded_file_path_new_name}
    ${unzipped_new_folder_name}=     Remove Spaces From Directory Name   ${DOWNLOAD_PATH}${TEST_FILE}[archive_name]
    # Validate output
    Compare Cheksum For Each File   ${unzipped_new_folder_name}    ${PATH_TO_PATTERN_FOLDER}
