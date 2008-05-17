

Object method!
initialize
	^ self!


"accessing"

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

Object method!
hash
	^ self identityHash!

Object method!
identityHash
	<primitive: 'Object_identityHash'>!


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


Object method!
notYetImplemented
	self error: 'This method is not yet implemented'!
Annotation key: 'category' value: 'error handling'!