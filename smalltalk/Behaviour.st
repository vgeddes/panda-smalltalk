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

instanceVariableNames
    "Answer a copy of the names of the instance variables"
    ^instanceVariableNames copy
!

printString
    "My string representation is my name itself"
    ^self name asString
!



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
!

subclass: classSymbol
    ^self subclass: classSymbol instanceVariableNames: '' classVariableNames: ''
!

subclass: classSymbol instanceVariableNames: instanceVariables
    ^self subclass: classSymbol instanceVariableNames: instanceVariables classVariableNames: ''
!

subclass: classSymbol instanceVariableNames: instanceVariableNames classVariableNames: classVariableNames
    | metaclass class |
    classSymbol first isUppercase
	ifFalse: [ self error: 'Class names must be capitalized' ].

    class := Smalltalk at: classSymbol ifAbsent: [
	metaclass := Metaclass new.
	class := metaclass initializeSubclassOf: self class named: nil.
	Smalltalk at: classSymbol put: class ].

    ^class
	initializeSubclassOf: self named: classSymbol;
	setInstanceVariableNames: instanceVariableNames;
	setClassVariableNames: classVariableNames;
	yourself
!

compile: aString
    | method |
    method := aString compileFor: self.
    methodDictionary
	ifNil: [ methodDictionary := Dictionary new: 10 ].
    methodDictionary at: method selector put: method
!

updateInstanceSize
    instanceSize := superclass instanceSize + instanceVariableNames size.
    self allSubclassesDo: [ :ea | ea updateInstanceSize ]
!

parseVariableNames: aString
    | lexer token coll |
    aString isString
	ifFalse: [ self error: 'Expected a string containing variable names' ].
    lexer := CompilerLexer text: aString.
    coll := OrderedCollection new.
    [ (token := lexer nextToken) notNil ]
	whileTrue: [
	    token class = CompilerIdentifierToken
		ifFalse: [ self error: 'Instance variables must be valid identifiers' ].
	    token first isLowercase
		ifFalse: [ self error: 'Instance variables must not be capitalized' ].
	    coll add: token asSymbol ].
    ^coll asArray
!

setInstanceVariableNames: aString
    instanceVariableNames := self parseVariableNames: aString.
    self updateInstanceSize
!


instanceSize
    ^instanceSize
!

initializeSubclassOf: aClass named: classSymbol
    instanceSize := aClass instanceSize.
    superclass := aClass.
    instanceVariableNames := #().
    subclasses := #().
    methodDictionary := IdentityDictionary new.
    name := classSymbol.
    aClass addSubclass: self
!

doesUnderstand: selector
    "Answer true if the receiver understand the given message selector, else false"
    | class |
    class := self.
    [ class notNil ]
	whileTrue: [
	    (class methodDictionary includesKey: selector)
		ifTrue: [ ^true ].
	    class := class superclass ].
    ^false
!

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
    class := self.
    [ (class := class superclass) notNil ]
	whileTrue: [ aBlock value: class ]
!



classVariables
    ^classVariables
!

setClassVariableNames: aString
    | array |
    array := self parseVariableNames: aString.
    classVariables := IdentityDictionary new: array size.
    array do: [ :ea |
	classVariables at: ea put: nil ]
!

