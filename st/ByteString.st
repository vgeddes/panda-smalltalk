
"accessing"

ByteString method!
at: anInteger
	^ self basicAt: anInteger!
Annotation key: 'category' value: 'accessing'!

ByteString method!
basicAt: anInteger
	<primitive: 'ByteString_at'>
	self primitiveFailed!
Annotation key: 'category' value: 'accessing'!

"testing"

String method!
isByteString
	^ true!
Annotation key: 'category' value: 'testing'!