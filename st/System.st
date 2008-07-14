
"accessing"

System method!
at: key put: object
	^ globals at: key put: object!

System method!
at: key
	^ globals at: key!

System method!
globals
	^ globals!

System method!
symbols
	^ symbols!

"system control"

System method!
startupSystem
	self exitWithResult: nil doIt printString!

System method!
exitWithResult: anObject
	<primitive: 'System_exitWithResult'>
	self primitiveFailed!

System method!
exit
	self exitWithResult: nil!