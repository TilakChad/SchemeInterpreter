(define (abs x)
	(if (< x 0)
		(- x)
		x
	))

(define (fn x N)
	(- (* x x) N))

(define (closesqrt m N)
	(< (abs (fn m N)) 0.005))

(define (avg x y)
	(/ (+ x y) 2))

(define (ispositive x) 
		(> x 0))

(define (bisection a b N)
		(let (
				(m (avg a b))
			 )
			 (if (closesqrt m N)
				 m
				 (if (ispositive (* (fn m N) (fn a N)))
					 (bisection m b N)
					 (bisection a m N)
				 )
			 )
		)
)

(bisection 1.0 5.0 8.0)

;; function to calculate the fixed point of a function by repeated application

(define (isclose x n)
		(< (abs (- x n)) 0.0005)
		)

(define (fixedpoint fn x)
		(if (isclose (fn x) x)
			x
			(fixedpoint fn (fn x))))
;; golden ratio phi
(fixedpoint (lambda (x) (+ 1 (/ 1.0 x))) 1.0)

