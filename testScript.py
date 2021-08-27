import subprocess
import time

stockfish = subprocess.Popen("./stockfish_14_x64_avx2",
                            universal_newlines=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            bufsize=1
                            )

custom = subprocess.Popen(["stdbuf", "-oL", "src/main"],
                            universal_newlines=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            bufsize=1
                            )

position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
# need to test this one, pretty sure it breaks things because pieces of the same color
# as the enemy cannont be on a threatend tile
# position = "8/8/6K1/7r/7k/8/8/8 w - - 0 1"
# position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
# position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
position ="rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
position = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 "
depth_max = 5
def put(dest, command):
    dest.stdin.write(command + '\n')

def get(src, toPrint=True, waitUntil="readyok", end=None):

    # using the 'isready' command to wich the engine must answer 'readyok'
    # which is then used to find last line of output
    
    size = 0
    put(src, 'isready')
    while True:
        out = src.stdout.readline().strip()
        if out == '' or waitUntil in out:
            break
        if out == 'readyok':
            continue
        size += 1
        if (toPrint):
            print(out)
        if (end != None):
            end.append(out)
    return size

def getResults(src):
    dest = []
    while(not get(src, toPrint=False, waitUntil="Nodes", end=dest)):
        continue
    dest = [s.replace(" ", "") for s in dest]
    dest.sort()
    return dest

def run(instr, depth):
    resultCustom = []
    resultStockfish = []
    put(custom, instr)
    get(custom, toPrint=False)

    put(stockfish, instr)
    get(stockfish, toPrint=False)

    put(stockfish, "go perft " + str(depth))
    put(custom, "go perft " + str(depth))

    resultStockfish = getResults(stockfish)
    resultCustom = getResults(custom)

    return resultStockfish, resultCustom

def searchErrors():

    depth = 0
    instr = "position fen " + position
    resultCustom = []
    resultStockfish = []
    while ( depth < depth_max and resultCustom == resultStockfish ) :
        depth += 1
        print("Looking at depth {}".format(depth))
        resultStockfish, resultCustom = run(instr, depth)
    if resultCustom !=  resultStockfish:
        findErrors(depth, results=[resultStockfish, resultCustom])
    else:
        print("All clear :)")


def findErrors(depth=1, moves=[], tabs=0, results=None):

    if depth == 0:
        return
    instr = "position fen " + position

    if moves:
        instr += " moves "
        for m in moves:
            tmp = m.split(':')
            instr += tmp[0] + " "

    if results:
        resultStockfish, resultCustom = results
    else:
        resultStockfish, resultCustom = run(instr, depth)

    resultCustom.sort()
    resultStockfish.sort()

    stockfishMoves = [m.split(':')[0] for m in resultStockfish]
    customMoves = [m.split(':')[0] for m in resultCustom]

    if len(resultCustom) < len(resultStockfish):
        missing = []
        for m in stockfishMoves:
            if m not in customMoves:
                missing.append(m)
        print("{}missing moves {}".format("\t"*tabs, missing))
    elif len(resultCustom) > len(resultStockfish):
        missing = []
        for m in customMoves:
            if m not in stockfishMoves:
                missing.append(m)
        print("{}illegal moves {}".format("\t"*tabs, missing))
    else:
        for i in range(len(resultCustom)):
            if resultCustom[i] == resultStockfish[i]:
                continue
            elif resultCustom[i].split(':')[0] != resultStockfish[i].split(':')[0]:
                print("Illegal Move : {}".format(resultCustom[i].split(':')[0]))
            else:
                string = "\t"*tabs
                m = resultCustom[i].split(':')[0]
                print(string + m)
                moves.append(m)
                findErrors(depth-1, moves, tabs+1)
            break

def main():
    get(stockfish, toPrint=False)
    put(stockfish, "uci")
    get(stockfish, toPrint=False)

    # put(custom, "isready")
    # get(custom, toPrint=False)

    searchErrors()

    put(stockfish, "quit")
    put(custom, "quit")

if __name__ == "__main__":
    main()
