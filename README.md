### rib

A LISP syntax compiler backend emitting LLVM.

:construction: Not currently usable for anything meaningful.

I am actively working on this, don't expect it to die any time soon.

```clj
(define square real real {
	(ret  (* % %))
})

(define main void int {
	(global arr [2 3 5 7 11])
	(global b (square (cast (index arr 3) real))) ; and we epically do nothing with this

	(print "stdout moment")

	(global strucha <1 "string">) ; structs too (no they're tuples)

	(ret 0)
})
```

compiles to

```llvm
; ModuleID = 'rib'
source_filename = "rib"

@arr = global <5 x i64> <i64 2, i64 3, i64 5, i64 7, i64 11>
@b = global double 0.000000e+00
@0 = private unnamed_addr constant [14 x i8] c"stdout moment\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"string\00", align 1
@strucha = global { i64, i8* } { i64 1, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @1, i32 0, i32 0) }

define double @square(double %0) {
entry:
  %1 = fmul double %0, %0
  ret double %1
}

define i64 @main() {
entry:
  %0 = load <5 x i64>, <5 x i64>* @arr, align 64
  %1 = extractelement <5 x i64> %0, i64 3
  %2 = sitofp i64 %1 to double
  %3 = call double @square(double %2)
  store double %3, double* @b, align 8
  %4 = call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @0, i32 0, i32 0))
  ret i64 0
}

declare i32 @puts(i8*)
```
