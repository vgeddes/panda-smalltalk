

Object method!
at: anIndex
	^ self!

Object method!
at: anIndex put: anObject
	^ self!

Object method!
basicAt: anIndex
	<primitive: 'Object_at'>!

Object method!
basicAt: anIndex put: anObject
	<primitive: 'Object_at_put'>!

Object method!
doesNotUnderstand: aMessage
	^ self!

Object method!
class
	<primitive: 'Object_class'>!
Annotation key: 'category' value: 'accessing'!

Object method!
hash
	^ self identityHash!
Annotation key: 'category' value: 'accessing'!

Object method!
identityHash
	<primitive: 'Object_identityHash'>!
Annotation key: 'category' value: 'accessing'!

Object method!
= anObject
	^ self == anObject!
Annotation key: 'category' value: 'testing'!

Object method!
== anObject
	<primitive: 'Object_equivalent'>!
Annotation key: 'category' value: 'testing'!