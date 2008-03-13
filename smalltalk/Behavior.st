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

!Behavior methodsFor: 'accessing'!

name
    "Answer my name"
    ^name
!

subclasses
    "A list of direct subclasses of the receiver"
    ^subclasses
!

superclass
    "Answer my superclass"
    ^superclass
!

methodDictionary
    "Returns the Dictionary containing the methods of the class"
    ^methodDictionary
!

printString
    "My string representation is my name itself"
    ^self name asString
! !

!Behavior methodsFor: 'instance creation'!

basicNew
    <primitive: 'Behavior_new'>
	^self primitiveFailed
!

new
    <primitive: 'Behavior_new'>
	^self primitiveFailed
!

basicNew: anInteger
    <primitive: 'Behavior_newColon'>
	^self primitiveFailed
!

new: anInteger
    self shouldNotImplement
! !

!Behavior methodsFor: 'class creation'!

subclass: classSymbol
    ^self subclass: classSymbol instanceVariableNames: '' classVariableNames: ''
!

subclass: classSymbol instanceVariableNames: instanceVariables
    ^self subclass: classSymbol instanceVariableNames: instanceVariables classVariableNames: ''
!

subclass: classSymbol instanceVariableNames: instanceVariables classVariableNames: classVariables
    | metaclass class |
    metaclass := Metaclass newSubclassOf: self class named: nil.
    class := metaclass newSubclassOf: self named: classSymbol.
    Smalltalk at: classSymbol put: class.
    ^class
! !

!Behavior methodsFor: 'private'!

instanceSize
    ^instanceSize
!

newSubclassOf: aClass named: classSymbol
    | class |
    class := self new.
    class initializeSubclassOf: aClass named: classSymbol.
    ^class
!

initializeSubclassOf: aClass named: classSymbol
    instanceSize := aClass instanceSize.
    superclass := aClass.
    instanceVariableNames := #().
    subclasses := #().
    name := classSymbol.
    aClass addSubclass: self
! !

!Behavior methodsFor: 'enumerating'!

addSubclass: aClass
    subclasses := subclasses copyWith: aClass
!

allSubclasses
    "Answer a list of all subclasses of the receiver"
    | ret |
    ret := OrderedCollection new.
    self allSubclassesDo: [ :ea |
	ret add: ea ].
    ^ret
!

allSubclassesDo: aBlock
    "Call aBlock recursing trough each subclass of this class"
    self subclasses do: [ :ea |
	aBlock value: ea.
	ea allSubclassesDo: aBlock ]
!

allSuperclassesDo: aBlock  
    "Call aBlock for each superclass of this class"
    | class |
    [ (class := self superclass) notNil ]
	whileTrue: [ aBlock value: class ]
! !

!Class methodsFor: 'initialize-release'!

initialize

! !
