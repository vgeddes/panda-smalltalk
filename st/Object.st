

Object method!
initialize
	^ self!


"accessing"

Object method!
class
	<primitive: 'Object_class'>
	self primitiveFailed!

Object method!
hash
	^ self identityHash!

Object method!
identityHash
	<primitive: 'Object_identityHash'>!

Object method!
yourself
	^ self!


"comparing"

Object method!
~= anObject
	^ self = anObject == false!

Object method!
= anObject
	^ self == anObject!

Object method!
~~ anObject
	^ self == anObject == false!

Object method!
== anObject
	<primitive: 'Object_equivalent'>!


"associating"
Object method!
-> anObject
    "Create a new association between myself and anObject"
	^ Association key: self value: anObject!




"printing"

Object method!
isVowel: aCharacter
	#($a $e $u $i $o)
		do: [ :c | c = aCharacter ifTrue: [^ true]].
	^ false!


Object method!
printOn: aStream
	| name |
    "Append to aStream a String representation of the receiver"

	name := self class name.

    aStream nextPutAll: ((self isVowel: name first) ifTrue: [ 'an' ] ifFalse: [ 'a' ])
	aStream nextPut: Character space.
    aStream nextPutAll: self class name!

Object method!
printString
    "Answer a String representation of the receiver"
    | stream |
    stream := WriteStream on: (WideString new: 20).
    self printOn: stream.
    ^ stream contents!


"testing"

Object method!
isNil
	^ false!

Object method!
isNotNil
	^ true!


"controlling"

Object method!
ifNil: alternativeBlock
	^ nil!

Object method!
ifNotNil: alternativeBlock
	^ alternativeBlock value!


"message handling"

Object method!
perform: aSymbol
	<primitive: 'Object_perform'>
	self primitiveFailed!

Object method!
perform: aSymbol with: anObject
	<primitive: 'Object_perform'>
	self primitiveFailed!

Object method!
perform: aSymbol with: firstObject with: secondObject
	<primitive: 'Object_perform'>
	self primitiveFailed!

Object method!
perform: aSymbol with: firstObject with: secondObject with: thirdObject
	<primitive: 'Object_perform'>
	self primitiveFailed!

Object method!
perform: aSymbol withArguments: anArray
	<primitive: 'Object_perform_withArguments'>
	self primitiveFailed!


"testing"

Object method!
isString
	^ false!

Object method!
isSymbol
	^ false!

Object method!
isCharacter
	^ false!

Object method!
isCollection
	^ false!

Object method!
isArray
	^ false!

Object method!
isNumber
	^ false!

Object method!
isInteger
	^ false!

Object method!
isFraction
	^ false!


"error handling"

Object method!
assert: aBlock
	aBlock value or: [ self error: 'assertion failed' ]!

Object method!
assert: aBlock description: aString
	aBlock value or: [ self error: aString ]! 

Object method!
error: aString 
	<primitive: 'Object_error'>!

Object method!
doesNotUnderstand: aMessage
	^ self error: aMessage selector!

Object method!
mustBeBoolean
	^ self error: 'must be boolean'!

Object method!
primitiveFailed 
	self error: 'primitive failed'!

Object method!
shouldNotImplement
	"Announce that, although the receiver inherits this message, it should 
	 not implement it."

	self error: 'This message is not appropriate for this object'!

Object method!
subclassResponsibility
	"This message sets up a framework for the behavior of the class' subclasses.
	 Announce that the subclass should have implemented this message."

	self error: 'My subclass should have overridden '!

Object method!
notYetImplemented
	self error: 'This method is not yet implemented'!


"copying"

Object method!
copy
	^ self basicCopy!

Object method!
basicCopy
	<primitive: 'Object_copy'>
	self primitiveFailed!


"private"

Object method!
species
	^ self class!