
"instance creation"

String classMethod!
new: sizeRequested
	"Creates an empty String"
	self == String
		ifTrue: [ ^ ByteString new: sizeRequested ]
		ifFalse: [ ^ (self basicNew: sizeRequested) initialize ]!

String classMethod!
newFromBytes: aByteArray
	"Creates an empty String"
	self == String
		ifTrue: [ ^ ByteString newFromBytes: aByteArray ]
		ifFalse: [ ^ self basicNew initializeWithBytes: aByteArray ]!



"comparing"

String method!
= aString
	aString isString
		ifFalse: [^ false].
	^ (self compare: aString) = 0!

String method!
< aString
	^ (self compare: aString) < 0!

String method!
> aString
	^ (self compare: aString) > 0!

String method!
<= aString
	^ (self > aString) not!

String method!
>= aString
	^ (self < aString) not!

String method!
compare: aString
	self subclassResponsibility!

String method!
compareOrdinal: aString
	self subclassResponsibility!

String method!
hash
	self subclassResponsibility!


"testing"

String method!
isString
	^ true!