

"instance creation"

Character classMethod!
new: unicodeValue
	^ self basicNew value: unicodeValue!
Annotation key: 'category' value: 'instance creation'!


"accessing"

Character method!
value
	^ value!
Annotation key: 'category' value: 'accessing'!


"private"

Character method!
value: unicodeValue
	value := unicodeValue!
Annotation key: 'category' value: 'private'!