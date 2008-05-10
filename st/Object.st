

Object method!
initialize
	^ self!

Object method!
at: anIndex
	<primitive: 'Object_at'>
	self primitiveFailed!

Object method!
at: anIndex put: anObject
	<primitive: 'Object_at_put'>
	self primitiveFailed!

Object method!
basicAt: anIndex
	<primitive: 'Object_at'>
	self primitiveFailed!

Object method!
basicAt: anIndex put: anObject
	<primitive: 'Object_at_put'>
	self primitiveFailed!

Object method!
basicSize
	<primitive: 'Object_size'>
	self primitiveFailed!

Object method!
doesNotUnderstand: aMessage
	self error: 'Object does not understand message'!

Object method!
class
	<primitive: 'Object_class'>
	self primitiveFailed!
Annotation key: 'category' value: 'accessing'!

Object method!
hash
	^ self identityHash!
Annotation key: 'category' value: 'accessing'!

Object method!
identityHash
	<primitive: 'Object_identityHash'>!
Annotation key: 'category' value: 'accessing'!


"comparing"

Object method!
= anObject
	^ self == anObject!
Annotation key: 'category' value: 'comparing'!

Object method!
~= anObject
	^ self = anObject == false!
Annotation key: 'category' value: 'comparing'!

Object method!
== anObject
	<primitive: 'Object_equivalent'>!
Annotation key: 'category' value: 'comparing'!


"testing"

Object method!
isNil
	^ false!
Annotation key: 'category' value: 'testing'!


"controlling"

Object method!
ifNil: alternativeBlock
	^ nil!
Annotation key: 'category' value: 'controlling'!

Object method!
ifNotNil: alternativeBlock
	^ alternativeBlock value!
Annotation key: 'category' value: 'controlling'!


"error handling"

Object method!
error: aString 
	<primitive: 'Object_error'>!
Annotation key: 'category' value: 'error handling'!

Object method!
primitiveFailed 
	self error: 'primitive failed'!
Annotation key: 'category' value: 'error handling'!

Object method!
shouldNotImplement
	"Announce that, although the receiver inherits this message, it should 
	 not implement it."

	self error: 'This message is not appropriate for this object'!
Annotation key: 'category' value: 'error handling'!

Object method!
subclassResponsibility
	"This message sets up a framework for the behavior of the class' subclasses.
	 Announce that the subclass should have implemented this message."

	self error: 'My subclass should have overridden '!
Annotation key: 'category' value: 'error handling'!