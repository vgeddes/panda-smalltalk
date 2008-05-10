

"testing"

UndefinedObject method!
isNil
	^ true!
Annotation key: 'category' value: 'testing'!


"controlling"

UndefinedObject method!
ifNil: alternativeBlock
	^ alternativeBlock value!
Annotation key: 'category' value: 'controlling'!

UndefinedObject method!
ifNotNil: alternativeBlock
	^ nil!
Annotation key: 'category' value: 'controlling'!