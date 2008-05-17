

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
