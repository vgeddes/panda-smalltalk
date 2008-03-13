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

!String class methodsFor: 'instance creation'!

new: anIndex
    "Add 1 to anIndex to make room for NULL character"
    ^super new: anIndex + 1
! !

!String methodsFor: 'basic'!

at: anIndex
    ^Character codePoint: (self byteAt: anIndex)
!

at: anIndex put: aCharacter
    aCharacter isCharacter
	ifFalse: [ self error: 'A character is required' ].
    self byteAt: anIndex put: aCharacter codePoint.
    ^aCharacter
!

basicAt: anIndex
    ^Character codePoint: (self byteAt: anIndex)
!

basicAt: anIndex put: aCharacter
    aCharacter isCharacter
	ifFalse: [ self error: 'A character is required' ].
    self byteAt: anIndex put: aCharacter codePoint.
    ^aCharacter
! !

!String methodsFor: 'primitives'!

printString
    ^self
!

compile
    <primitive: 'String_compile'>
	self primitiveFailed
! !

!String methodsFor: 'basic'!

hash
    <primitive: 'String_hash'>
	self primitiveFailed
!

size
    "The size is less by 1 because of the NULL character"
    ^self basicSize - 1
! !

!String methodsFor: 'utilities'!

doIt
    | method context |
    method := ('goDoIt ', self) compile.
    context := MethodContext on: method parent: thisContext receiver: nil arguments: nil.
    ^Processor enter: context
! !

!String methodsFor: 'converting'!

asSymbol
    <primitive: 'String_asSymbol'>
	self primitiveFailed
!

asString
    ^self
! !

!String methodsFor: 'testing'!

isSymbol
    ^false
!

isString
    ^true
! !
