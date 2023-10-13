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


*** Test Cases ***
Find and open About page
    [documentation]  Click button About test and check if subpage is opened
    [tags]  Regression
    #TODO: Find button About test, clicks it and verifies if a list with some text dislays
    Click Element  ${AboutButton}
    Wait Until Element Is Visible   ${InnoextractText}    1


    