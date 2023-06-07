*** Settings ***
Documentation       Test files data

*** Variables ***
${test_files_directory_path}  ${CURDIR}${/}
&{file_4mb}  name=file_4MB.exe  path=${test_files_directory_path}file_4MB.exe  archive_size_bytes=${4722731}
&{10k_files}  name=10k_files.exe  path=${test_files_directory_path}10k_files.exe  archive_size_bytes=${42537871}
