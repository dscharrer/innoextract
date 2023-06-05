*** Settings ***
Library  SeleniumLibrary
Variables  ../locators/locators.py
Variables  ../test_data/test_data.py

*** Keywords ***
Click Add Files Button
    Click Element  ${AddFilesButton}

Click Extract And SaveButton
    Click Element  ${ExtractAndSaveButton}

Click Remove Button
    Click Element  ${RemoveButton}

Click Start Button
    Click Element  ${StartButton}
