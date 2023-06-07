*** Settings ***
Library  AutoItLibrary
Library    String

*** Keywords ***
Upload Test File
    [Arguments]  ${file_path}
    ${window_title}  Win Get Title  [CLASS:#32770]  ${EMPTY}
    Control Focus  [CLASS:#32770]  ${EMPTY}  [CLASSNN:Edit1]
    Log  Upload file: ${file_path}  console=yes
    Control Set Text  [CLASS:#32770]  ${EMPTY}  [CLASSNN:Edit1]  ${file_path} 
    Control Click  [CLASS:#32770]  ${EMPTY}  &Open
    Win Wait Close  ${window_title}  Timeout=${3}
