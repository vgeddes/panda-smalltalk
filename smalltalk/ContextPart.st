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

!MethodContext methodsFor: 'testing'!

showTraceback
    "Shows my single traceback"
    (self receiver class printString, '>>', self method selector printString) printNl
!

handles: anException
    ^false
!

homeContext
    ^self
!

isMethodContext
    ^true
! !

!ContextPart class methodsFor: 'instance creation'!

on: aMethod parent: aContext receiver: anObject arguments: anArray
    "Create a new execution context"
    ^self new
	method: aMethod
	parent: aContext
	receiver: anObject
	arguments: anArray;
	yourself
! !

!ContextPart methodsFor: 'debugging'!

handles: anException
    self subclassResponsibility
!

showTraceback
    self subclassResponsibility
!

homeContext
    self subclassResponsibility
!

traceback
    "Recurse trough my parent contexts and show their tracebacks"
    | context |
    context := self.
    [ context notNil ]
	whileTrue: [
	    context showTraceback.
	    context := context parent ]
! !


!ContextPart methodsFor: 'accessing'!

isMethodContext
    self subclassResponsibility
! !

!ContextPart methodsFor: 'accessing'!

method
    ^method
!

method: aMethod parent: aContext receiver: anObject arguments: anArray
    "if method is not nil then this context has been initialized"
    method notNil ifTrue: [^self].

    method := aMethod.
    returnContext := parent := aContext.
    receiver := anObject.

    method argumentCount > 0
	ifTrue: [ arguments := Array new: method argumentCount ].

    anArray notNil
	ifTrue: [ arguments replaceFrom: 1 with: anArray ].

    method temporaryCount > 0
	ifTrue: [ temporaries := Array new: method temporaryCount ].

    ip := sp := 0.
    stack := Array new: method stackSize
!

arguments
    ^arguments
!

receiver
    ^receiver
!

parent: aContext
    parent := aContext
!

parent
    ^parent
! !
