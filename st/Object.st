

Object methodFor: ''!
at: anIndex
	^ self
!

Object methodFor: ''!
at: anIndex put: anObject
	^ self
!

Object methodFor: ''!
basicAt: anIndex
	<primitive: 'Object_at'>
!

Object methodFor: ''!
basicAt: anIndex put: anObject
	<primitive: 'Object_at_put'>
!

Object methodFor: ''!
doesNotUnderstand: aMessage
	^ self
!

Object methodFor: 'accessing'!
class
	<primitive: 'Object_class'>
!

Object methodFor: 'accessing'!
hash
	^ self identityHash
!

Object methodFor: 'accessing'!
identityHash
	<primitive: 'Object_identityHash'>
!

Object methodFor: 'testing'!
= anObject
	^ self == anObject 
!

Object methodFor: 'testing'!
== anObject
	<primitive: 'Object_equivalent'>
!