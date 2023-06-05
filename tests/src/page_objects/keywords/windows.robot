*** Settings ***
Library  AutoItLibrary

*** Keywords ***
Upload File
    [Arguments]  ${file_path}
    Control Focus  [CLASS:#32770]  ${EMPTY}  [CLASSNN:Edit1]
    Control Send  [CLASS:#32770]  ${EMPTY}  [CLASSNN:Edit1]  ${file_path}
    Control Click  [CLASS:#32770]  ${EMPTY}  &Open