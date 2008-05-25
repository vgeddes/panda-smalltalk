

"testing"

UndefinedObject method!
isNil
	^ true!

UndefinedObject method!
isNotNil
	^ false!


"controlling"

UndefinedObject method!
ifNil: alternativeBlock
	^ alternativeBlock value!

UndefinedObject method!
ifNotNil: alternativeBlock
	^ nil!


"system"

UndefinedObject method!
startupSystem
	self exitWithResult: self doIt!

UndefinedObject method!
exitWithResult: anObject
	<primitive: 'UndefinedObject_exitWithResult'>
	self primitiveFailed!

UndefinedObject method!
exit
	self exitWithResult: nil!
