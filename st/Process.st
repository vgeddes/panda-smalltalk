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

!Process class methodsFor: 'instance creation'!

on: aContext
    ^self new context: aContext; initialize; yourself
! !

!Process methodsFor: 'execution'!

initialize
    self suspend
!

context
    ^context
!

context: aContext
    context := aContext
!

suspend
    suspended := true.
    self isActive ifTrue: [Processor yield]
!

resume
    suspended := false.
    self isActive ifFalse: [Processor yield]
!

terminate
    self isActive
	ifTrue: [ Processor leaveTo: nil andAnswer: nil]
	ifFalse: [ context := nil ]
! !

!Process methodsFor: 'testing'!

isActive
    "Answer if the receiver is the active process being run"
    ^self == Processor activeProcess
! !
