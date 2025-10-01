This project consists in an upgrade from my previous train itinerary calculator, written for a school project in python. 
The entire core algorithm was reimplemented in C++ with appropriate data structures and more efficient algorithms. As a result, it is much faster.
The architecture was rebuilt using a client-server design, with a python GUI conversing with the C++ server using sockets. 
The algorithm itself is well explained in the code, and constituted an intersting problem dealing with shortests paths in a time-evolving graph.

All data used is open-source and can be found at https://www.data.gouv.fr/datasets/horaires-des-tgv/
