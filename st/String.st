
"instance creation"

String classMethod!
new: sizeRequested
	"Creates an empty String"
	self == String
		ifTrue: [ ^ ByteString new: sizeRequested ]
		ifFalse: [ ^ (self basicNew: sizeRequested) initialize ]!
Annotation key: 'category' value: 'instance creation'!


"testing"

String method!
isString
	^ true!
Annotation key: 'category' value: 'testing'!