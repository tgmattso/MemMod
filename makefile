# 
include make.def

#
# I have an EXES that omits sync_ov3 since that doesn't compile
# 
EXES= sync_ov1$(EXE) sync_ov2$(EXE) sync_ov0$(EXE) 
#EXES= sync_ov1$(EXE) sync_ov2$(EXE) sync_ov3$(EXE) sync_ov0$(EXE) 

all: $(EXES)

sync_ov0$(EXE): sync_ov0.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o sync_ov0 sync_ov0.$(OBJ) $(LIBS)

sync_ov1$(EXE): sync_ov1.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o sync_ov1 sync_ov1.$(OBJ) $(LIBS)

sync_ov2$(EXE): sync_ov2.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -o sync_ov2 sync_ov2.$(OBJ) $(LIBS)

sync_ov3$(EXE): sync_ov3.$(OBJ)
	$(CLINKER) $(OPTFLAGS) -std=c11 -o sync_ov3 sync_ov3.$(OBJ) $(LIBS)

test: $(EXES)
	$(PRE)sync_ov0$(EXE)  
	$(PRE)sync_ov1$(EXE)  
	$(PRE)sync_ov2$(EXE)  
	$(PRE)sync_ov3$(EXE)  

clean:
	$(RM) $(EXES) *.$(OBJ)

.SUFFIXES:
.SUFFIXES: .c .cpp  .$(OBJ)

.c.$(OBJ):
	$(CC) $(CFLAGS) -c $<

.cpp.$(OBJ):
	$(CC) $(CFLAGS) -c $<
