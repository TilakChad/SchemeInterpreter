;; ignores the comments
;; new comment

(define (f n) 
	(if (< n 3)
		n
		(+ (f (- n 1)) (* 2 (f (- n 2))) (* 3 (f (- n 3))))
	)
)

(define (flinear a b c n)
	(if (= n 0)  
		a
		(flinear (+ a (* 2 b) (* 3 c)) a b (- n 1))
	)
)


(define (fn n)
	(if (< n 3) 
		n
		(flinear 2 1 0 (- n 2))
	)
)

;; Exponentiation

;; (fn 10)

(define (square x) (* x x ))

(define (isEven x)
	(= (remainder x 2)  0)
)

(define (fastexp n x) 
	(define (internexp n i)
		(cond ((= i 1) n)
			  ((isEven i) (square (internexp n (/ i 2))))
			  (else (* n (internexp n (- i 1))))
		)
	)
	(internexp n x)
)

(fastexp 5 5)

;; Exercise 1.12 -> Pascal Triangle

(define (pascal n x)
		(cond ((= x 0) 1)
			  ((= n x) 1)
			  (else (+ 
						(pascal (- n 1) (- x 1))
						(pascal (- n 1) x)
						)
			  )
		)
)

(pascal 4 2)

;; Exercise 1.13 
;; phi and shi both follow x^2 = x + 1 relation, so using that Fib(n+1) = (phi^(n+1)  -  psi^(n+1)) / sq.5 can be shown easily 
;; To show   that Fib(n) is closest integer to (phi^n)/sq.5, let s = sqrt(5) 
;; is equivalent to showing |Fib(n) - (phi ^ n)/s| < 1/2
;; using Fib(n) = (phi ^ n) / s - (psi ^ n) / s, 
;;							|(psi^n) / s| <  1/2 
;;                          |((1 - sqrt(5))/2)^n| < sqrt(5)/2
;;							sqrt(5) < 3 -> sqrt(5) -  1 < 2 -> (sqrt(5) - 1) ^ n < 2 ^ n 
;;							This completes the proof :  above follow  that phi ^ n < 1 -> (phi ^ n) / sqrt(5) < 1/sqrt(5) < 1/2  
							