' Windows Installer utility to report the language and codepage for a package
' For use with Windows Scripting Host, CScript.exe or WScript.exe
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates the access of language and codepage values                 
'
Option Explicit

Const msiOpenDatabaseModeReadOnly     = 0
Const msiOpenDatabaseModeTransact     = 1
Const ForReading = 1
Const ForWriting = 2
Const TristateFalse = 0

Const msiViewModifyInsert         = 1
Const msiViewModifyUpdate         = 2
Const msiViewModifyAssign         = 3
Const msiViewModifyReplace        = 4
Const msiViewModifyDelete         = 6

Dim argCount:argCount = Wscript.Arguments.Count
If argCount > 0 Then If InStr(1, Wscript.Arguments(0), "?", vbTextCompare) > 0 Then argCount = 0
If (argCount = 0) Then
	message = "Windows Installer utility to manage language and codepage values for a package." &_
		vbNewLine & "The package language is a summary information property that designates the" &_
		vbNewLine & " primary language and any language transforms that are available, comma delim." &_
		vbNewLine & "The ProductLanguage in the database Property table is the language that is" &_
		vbNewLine & " registered for the product and determines the language used to load resources." &_
		vbNewLine & "The codepage is the ANSI codepage of the database strings, 0 if all ASCII data," &_
		vbNewLine & " and must represent the text data to avoid loss when persisting the database." &_
		vbNewLine & "The 1st argument is the path to MSI database (installer package)" &_
		vbNewLine & "To update a value, the 2nd argument contains the keyword and the 3rd the value:" &_
		vbNewLine & "   Package  {base LangId optionally followed by list of language transforms}" &_
		vbNewLine & "   Product  {LangId of the product (could be updated by language transforms)}" &_
		vbNewLine & "   Codepage {ANSI codepage of text data (use with caution when text exists!)}" &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Echo message
	Wscript.Quit 1
End If

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError


' Open database
Dim databasePath:databasePath = Wscript.Arguments(0)
Dim openMode : If argCount >= 3 Then openMode = msiOpenDatabaseModeTransact Else openMode = msiOpenDatabaseModeReadOnly
Dim database : Set database = installer.OpenDatabase(databasePath, openMode) : CheckError

' Update value if supplied
If argCount >= 3 Then
	Dim value:value = Wscript.Arguments(2)
	Select Case UCase(Wscript.Arguments(1))
		Case "PACKAGE"  : SetPackageLanguage database, value
		Case "PRODUCT"  : SetProductLanguage database, value
		Case "CODEPAGE" : SetDatabaseCodepage database, value
		Case Else       : Fail "Invalid value keyword"
	End Select
	CheckError
End If

' Extract language info and compose report message
Dim message:message = "Package language = "         & PackageLanguage(database) &_
					", ProductLanguage = " & ProductLanguage(database) &_
					", Database codepage = "        & DatabaseCodepage(database)
database.Commit : CheckError  ' no effect if opened ReadOnly
Set database = nothing
Wscript.Echo message
Wscript.Quit 0

' Get language list from summary information
Function PackageLanguage(database)
	On Error Resume Next
	Dim sumInfo  : Set sumInfo = database.SummaryInformation(0) : CheckError
	Dim template : template = sumInfo.Property(7) : CheckError
	Dim iDelim:iDelim = InStr(1, template, ";", vbTextCompare)
	If iDelim = 0 Then template = "Not specified!"
	PackageLanguage = Right(template, Len(template) - iDelim)
	If Len(PackageLanguage) = 0 Then PackageLanguage = "0"
End Function

' Get ProductLanguge property from Property table
Function ProductLanguage(database)
	On Error Resume Next
	Dim view : Set view = database.OpenView("SELECT `Value` FROM `Property` WHERE `Property` = 'ProductLanguage'")
	view.Execute : CheckError
	Dim record : Set record = view.Fetch : CheckError
	If record Is Nothing Then ProductLanguage = "Not specified!" Else ProductLanguage = record.IntegerData(1)
End Function

' Get ANSI codepage of database text data
Function DatabaseCodepage(database)
	On Error Resume Next
	Dim WshShell : Set WshShell = Wscript.CreateObject("Wscript.Shell") : CheckError
	Dim tempPath:tempPath = WshShell.ExpandEnvironmentStrings("%TEMP%") : CheckError
	database.Export "_ForceCodepage", tempPath, "codepage.idt" : CheckError
	Dim fileSys : Set fileSys = CreateObject("Scripting.FileSystemObject") : CheckError
	Dim file : Set file = fileSys.OpenTextFile(tempPath & "\codepage.idt", ForReading, False, TristateFalse) : CheckError
	file.ReadLine ' skip column name record
	file.ReadLine ' skip column defn record
	DatabaseCodepage = file.ReadLine
	file.Close
	Dim iDelim:iDelim = InStr(1, DatabaseCodepage, vbTab, vbTextCompare)
	If iDelim = 0 Then Fail "Failure in codepage export file"
	DatabaseCodepage = Left(DatabaseCodepage, iDelim - 1)
	fileSys.DeleteFile(tempPath & "\codepage.idt")
End Function

' Set ProductLanguge property in Property table
Sub SetProductLanguage(database, language)
	On Error Resume Next
	If Not IsNumeric(language) Then Fail "ProductLanguage must be numeric"
	Dim view : Set view = database.OpenView("SELECT `Property`,`Value` FROM `Property`")
	view.Execute : CheckError
	Dim record : Set record = installer.CreateRecord(2)
	record.StringData(1) = "ProductLanguage"
	record.StringData(2) = CStr(language)
	view.Modify msiViewModifyAssign, record : CheckError
End Sub

' Set ANSI codepage of database text data
Sub SetDatabaseCodepage(database, codepage)
	On Error Resume Next
	If Not IsNumeric(codepage) Then Fail "Codepage must be numeric"
	Dim WshShell : Set WshShell = Wscript.CreateObject("Wscript.Shell") : CheckError
	Dim tempPath:tempPath = WshShell.ExpandEnvironmentStrings("%TEMP%") : CheckError
	Dim fileSys : Set fileSys = CreateObject("Scripting.FileSystemObject") : CheckError
	Dim file : Set file = fileSys.OpenTextFile(tempPath & "\codepage.idt", ForWriting, True, TristateFalse) : CheckError
	file.WriteLine ' dummy column name record
	file.WriteLine ' dummy column defn record
	file.WriteLine codepage & vbTab & "_ForceCodepage"
	file.Close : CheckError
	database.Import tempPath, "codepage.idt" : CheckError
	fileSys.DeleteFile(tempPath & "\codepage.idt")
End Sub     

' Set language list in summary information
Sub SetPackageLanguage(database, language)
	On Error Resume Next
	Dim sumInfo  : Set sumInfo = database.SummaryInformation(1) : CheckError
	Dim template : template = sumInfo.Property(7) : CheckError
	Dim iDelim:iDelim = InStr(1, template, ";", vbTextCompare)
	Dim platform : If iDelim = 0 Then platform = ";" Else platform = Left(template, iDelim)
	sumInfo.Property(7) = platform & language
	sumInfo.Persist : CheckError
End Sub

Sub CheckError
	Dim message, errRec
	If Err = 0 Then Exit Sub
	message = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then message = message & vbNewLine & errRec.FormatText
	End If
	Fail message
End Sub

Sub Fail(message)
	Wscript.Echo message
	Wscript.Quit 2
End Sub
