

"logical operations"

False method!
& aBoolean
	^ false!
Annotation key: 'category' value: 'logical operations'!

False method!
| aBoolean
	^ aBoolean!
Annotation key: 'category' value: 'logical operations'!

False method!
not
	^ true!
Annotation key: 'category' value: 'logical operations'!


"controlling"

False method!
ifTrue: trueBlock
	^ nil!
Annotation key: 'category' value: 'controlling'!

False method!
ifFalse: falseBlock
	^ falseBlock value!
Annotation key: 'category' value: 'controlling'!

False method!
ifTrue: trueBlock ifFalse: falseBlock
	^ falseBlock value!
Annotation key: 'category' value: 'controlling'!

False method!
ifFalse: falseBlock ifTrue: trueBlock
	^ falseBlock value!
Annotation key: 'category' value: 'controlling'!

False method!
and: aBlock
	^ false!
Annotation key: 'category' value: 'controlling'!

False method!
or: aBlock
	^ aBlock value!
Annotation key: 'category' value: 'controlling'!