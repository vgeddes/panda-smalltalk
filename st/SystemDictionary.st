"
Copyright (c) 2007 Luca Bruno

This file is part of Smalltalk YX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

!SystemDictionary methodsFor: 'initialize-release'!

at: aSymbol put: anObject
    aSymbol isSymbol
	ifFalse: [ self error: 'system dictionary may have only symbol keys' ].
    ^super at: aSymbol put: anObject
!

quit: exitStatus
    "Quit Syx with exitStatus"
    <primitive: 'Smalltalk_quit'>
	self primitiveFailed
!

quit
    "Quit Syx"
    ^self quit: 0
!

initializeSystem
    "This method is called once when creating the image"
    self
	at: #stdin put: (FileStream on: 0);
	at: #stdout put: (FileStream on: 1);
	at: #stderr put: (FileStream on: 2);
	at: #Transcript put: (TextCollector proxyMessage: stdout -> #nextPutAll:).

    Object allSubclassesDo: [ :ea |
	ea initialize ]
! !

!SystemDictionary methodsFor: 'plugins'!

loadPlugin: name
    <primitive: 'Smalltalk_loadPlugin'>
	self primitiveFailed
!

unloadPlugin: name
    <primitive: 'Smalltalk_unloadPlugin'>
	self primitiveFailed
! !
