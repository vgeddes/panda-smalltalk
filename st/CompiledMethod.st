
"accessing"

CompiledMethod method!
header
	^ header!

CompiledMethod method!
literals
	^ literals!

CompiledMethod method!
bytecode
	^ bytecode!

CompiledMethod method!
selector
	^ selector!



CompiledMethod method!
methodClass
	^ literals isNil
		ifTrue: [nil]
		ifFalse: [literals at: literals size]!

"header"

CompiledMethod method!
flags
	^ (header bitShift: -27) bitAnd: 16r7!

CompiledMethod method!
argumentCount
	^ (header bitShift: -22) bitAnd: 16r1F!

CompiledMethod method!
temporaryCount
	^ (header bitShift: -16) bitAnd: 16r3F!

CompiledMethod method!
stackDepth
	^ (header bitShift:  -8) bitAnd: 16rFF!

CompiledMethod method!
primitive
	^ (header bitShift:   0) bitAnd: 16rFF!