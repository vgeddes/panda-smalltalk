
"instance creation"

String classMethod!
new: sizeRequested
	"Creates an empty String"
	self == String
		ifTrue: [ ^ ByteString new: sizeRequested ]
		ifFalse: [ ^ (self basicNew: sizeRequested) initialize ]!


"testing"

String method!
isString
	^ true!