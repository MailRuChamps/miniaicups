const readline = require('readline')

const REQUEST_MAX_TIME = 5
const MAX_EXECUTION_TIME = 120
const NS_PER_SEC = 1e9
let timeLimit = MAX_EXECUTION_TIME * NS_PER_SEC
let startTime

const debugFile = 'C:/temp/debug.txt'
const isDebug = process.argv.includes('--debug')
let tick = 0
let c = { GAME_TICKS: 1500 }
const maxDeep = 9

let FreeCells = []

if (isDebug) {
    require('fs').writeFileSync(debugFile, JSON.stringify({ isDebug }, null, 2))
}

function inPoly (x, y, pathX, pathY) {
    let cc = false
    try {
        pathX.reduce((p, c, idx) => {
            const pX = p[0]
            const pY = p[1]
            const cX = c
            const cY = pathY[idx]
            if ((((cY <= y) && (y < pY)) || ((pY <= y) && (y < cY))) &&
                (x > (pX - cX) * (y - cY) / (pY - cY) + cX)) {
                cc = !cc
            }
            return [cX, cY]
        }, [pathX[pathX.length - 1], pathY[pathY.length - 1]])
    } catch (e) {
    }
    return cc
}

const Debug = msg => {
    if (!isDebug) return
    require('fs').appendFileSync(debugFile, '\r' + tick.toString() + JSON.stringify(msg, null, 2))
}

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
})

const isBorder = ([x, y]) =>
    x < Math.round(c.width / 2) ||
    x >= c.x_cells_count * c.width + Math.round(c.width / 2) ||
    y < Math.round(c.width / 2) ||
    y >= c.y_cells_count * c.width + Math.round(c.width / 2)

const getNextPoint = ([x, y], command, step = 1) => {
    switch (command) {
    case 'up': return [x, y + c.width * step]
    case 'down': return [x, y - c.width * step]
    case 'left': return [x - c.width * step, y]
    case 'right': return [x + c.width * step, y]
    default: return [x, y]
    }
}

const isEmptyNextPoint = (player, command) => {
    const [x, y] = getNextPoint(player.position, command)
    return !player.linesMap[x + '_' + y] && !isBorder([x, y])
}

const isMyTerritory = (world, playerID, [x, y] = world.players[playerID].position) =>
    world.cells[x + '_' + y] === playerID

const isMyTerritoryNextPoint = (world, playerID, command) =>
    isMyTerritory(world,
        playerID,
        getNextPoint(world.players[playerID].position, command)
    )

const directionFilter = (command, player) => {
    if (!isEmptyNextPoint(player, command)) return false
    switch (player.direction) {
    case 'up': return command !== 'down'
    case 'down': return command !== 'up'
    case 'left': return command !== 'right'
    case 'right': return command !== 'left'
    default: return true
    }
}

const nearest = ({ position, cells, logging = false }) => {
    const [x, y] = position
    const pCoord = -2
    cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = pCoord
    let direction = false
    let ArrChanged = true
    let distance = 1
    while (ArrChanged && !direction) {
        ArrChanged = false
        for (let x = 1; x <= c.x_cells_count; x += 1) {
            for (let y = 1; y <= c.y_cells_count; y += 1) {
                if (cells[x][y] !== distance) continue
                if (cells[x + 1][y] === pCoord) direction = 'left'
                if (!cells[x + 1][y]) {
                    cells[x + 1][y] = distance + 1
                    ArrChanged = true
                }
                if (cells[x - 1][y] === pCoord) direction = 'right'
                if (!cells[x - 1][y]) {
                    cells[x - 1][y] = distance + 1
                    ArrChanged = true
                }
                if (cells[x][y + 1] === pCoord) direction = 'down'
                if (!cells[x][y + 1]) {
                    cells[x][y + 1] = distance + 1
                    ArrChanged = true
                }
                if (cells[x][y - 1] === pCoord) direction = 'up'
                if (!cells[x][y - 1]) {
                    cells[x][y - 1] = distance + 1
                    ArrChanged = true
                }
            }
        }
        distance += 1
    }
    // Debug({ distance, direction })
    if (logging) {
        cells.forEach(c => {
            Debug(c.join(''))
        })
    }
    return { distance, direction }
}

const distAndDirToHome = (world, playerID, logging = false) => {
    if (Object.keys(world.players[playerID].linesMap).length === 0) {
        return {
            distance: 0,
            direction: null
        }
    }
    const cells = Array(c.x_cells_count + 2).fill(0).map((_, i) => [...FreeCells[i]])
    for (let [key, value] of Object.entries(world.cells)) {
        if (value !== playerID) continue
        const [x, y] = key.split('_').map(Number)
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = 1
    }

    for (let key of Object.keys(world.players[playerID].linesMap)) {
        const [x, y] = key.split('_').map(Number)
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = -1
    }
    if (Object.keys(world.players[playerID].linesMap).length === 1) {
        let [x, y] = getNextPoint(world.players[playerID].position, world.players[playerID].direction, -1)
        // Debug({ x, y, position: world.players[playerID].position })
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = -1
    }
    if (logging) {
        
        cells.forEach(c => {
            Debug(c.join(''))
        })
    }
    return nearest({
        position: world.players[playerID].position,
        cells,
        logging
    })
}

const distAndDirToLines = (world, myID, enemyID) => {
    if (!Object.keys(world.players[enemyID].linesMap).length) {
        return {
            distance: 1000000,
            direction: null
        }
    }
    const cells = Array(c.x_cells_count + 2).fill(0).map((_, i) => [...FreeCells[i]])
    for (let key of Object.keys(world.players[enemyID].linesMap)) {
        const [x, y] = key.split('_').map(Number)
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = 1
    }
    for (let key of Object.keys(world.players[myID].linesMap)) {
        const [x, y] = key.split('_').map(Number)
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = -1
    }
    let [x, y] = getNextPoint(world.players[myID].position, world.players[myID].direction, -1)
    cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = -1
    return nearest({
        position: world.players[myID].position,
        cells
    })
}

const deadlyDirection = (world, playerID, command) => {
    const [x, y] = getNextPoint(world.players[playerID].position, command)
    const meMap = { ...world.players[playerID].linesMap }
    meMap[x + '_' + y] = true
    for (const enemyID of Object.keys(world.players)) {
        if (enemyID === playerID) continue
        const [ex, ey] = getNextPoint(world.players[enemyID].position, world.players[enemyID].direction)
        if (world.cells[ex + '_' + ey] === enemyID && world.players[enemyID].linesMap[x + '_' + y]) return playerID
        if (meMap[ex + '_' + ey]) return playerID
        if (world.players[enemyID].linesMap[x + '_' + y]) return enemyID
        // const enemyMap = {...world.players[enemyID].linesMap}
        // enemyMap[ex + '_' + ey] = true

        // if (world.players[enemyID].position[0] === x && world.players[enemyID].position[0] === y) {
        //     if (Object.keys(world.players[enemyID].linesMap).length > Object.keys(meMap).length) return enemyID
        //     else return playerID
        // }
        // if (world.players[enemyID].linesMap[x + '_' + y]) {
        //     if (playerID === 'i') {
        //         Debug({ enemyL: world.players[enemyID].linesMap, meL: world.players[playerID].linesMap })
        //     }
        //     return enemyID
        // }
    }
    return false
}

const momentaryKill = (world, playerID) => {
    const player = world.players[playerID]
    const ways = ['left', 'up', 'right', 'down'].filter(command => directionFilter(command, player))
    if (!ways.length) {
        return null // dead
    }
    const command = ways.find(command => {
        const killID = deadlyDirection(world, playerID, command)
        // if (killID) Debug({killID})
        if (killID && killID !== playerID) return true
        return false
    })
    if (!command) return null
    return command
}

let isEscape = false
const hunting = (world, myID) => {
    let bestDistToEnemy = 1000000
    let hunterDir = false
    isEscape = false
    let minDistanceToMe = 1000000
    const { distance: distToHome, direction: dirToHome } = distAndDirToHome(world, myID)
            
    Object.keys(world.players).filter(p => p !== myID).forEach(playerID => {
        if (isEscape) return
        let { distance: distToMe } = distAndDirToLines(world, playerID, myID)
        minDistanceToMe = Math.min(minDistanceToMe, distToMe)
        let { distance: distToEnemy, direction: dirToEnemy } = distAndDirToLines(world, myID, playerID)
        if (distToEnemy > distToMe && distToHome > distToMe / 4 // && distToHome > maxDeep
            ) {
            // go home
            hunterDir = dirToHome
            // if (dirToHome === 'right' && world.players.i.direction === 'left') {
            //     Debug({ distToHome, dirToHome, direction: world.players.i.direction })
            //     distAndDirToHome(world, 'i', true)
            // }
            isEscape = true
        }
        if (isEscape) return
        if (distToMe > distToEnemy && bestDistToEnemy > distToEnemy) {
            const { distance: distToHomeEnemy } = distAndDirToHome(world, playerID)
            if (distToHomeEnemy <= distToEnemy) {
                hunterDir = dirToEnemy
                bestDistToEnemy = distToEnemy
            }
        }
    })
    if (!isEscape && hunterDir) {
        if (bestDistToEnemy >= minDistanceToMe) return false
    }
    return hunterDir
}

const huntingFast = (world, myID) => {
    let hunterDir = false

    // const { distance: distToHome, direction: dirToHome } = distAndDirToHome(world, myID)
    const cells = Array(c.x_cells_count + 2).fill(0).map((_, i) => [...FreeCells[i]])
    Object.keys(world.players).filter(p => p !== myID).forEach(playerID => {
        for (let key of Object.keys(world.players[playerID].linesMap)) {
            const [x, y] = key.split('_').map(Number)
            cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = 1
            hunterDir = true
        }
    })
    for (let key of Object.keys(world.players[myID].linesMap)) {
        const [x, y] = key.split('_').map(Number)
        cells[Math.floor(x / c.width + 1)][Math.floor(y / c.width + 1)] = -1
    }
    if (!hunterDir) return null
    const { direction } = nearest({
        position: world.players[myID].position,
        cells
    })
    return direction
}

const continueDirection = (world, playerID) => {
    const player = world.players[playerID]
    const ways = ['left', 'up', 'right', 'down'].filter(command => directionFilter(command, player))
    if (!ways.length) {
        return player.direction // dead
    }
    const momentaryBenefit = momentaryKill(world, playerID)
    if (momentaryBenefit) return momentaryBenefit
    const hunterDir = huntingFast(world, playerID)
    if (hunterDir) return hunterDir
    if (isEmptyNextPoint(player, player.direction)) {
        return player.direction
    }
    return ways[0]
}

const calcSeizureOfTerritory = (world, playerID, command) => {
    const player = world.players[playerID]
    if (!player.lines.length) return 0
    const path = [...player.lines, getNextPoint(player.position, command)]
    const pathX = path.map(p => p[0])
    const pathY = path.map(p => p[1])
    const minX = Math.min(...pathX)
    const minY = Math.min(...pathY)
    const maxX = Math.max(...pathX)
    const maxY = Math.max(...pathY)
    let bonus = 0
    // return player.lines.length
    for (let x = minX; x <= maxX; x += c.width) {
        for (let y = minY; y <= maxY; y += c.width) {
            if (inPoly(x, y, pathX, pathY)) {
                let info = world.cells[x + '_' + y]
                if (info) {
                    if (info !== playerID) bonus += 5
                } else {
                    bonus++ // empty cell
                }
                world.cells[x + '_' + y] = playerID
            }
        }
    }
    return bonus
}

const sumBonuses = b => {
    // Debug(b)
    return b.reduce((p, c) => p + c)
}

const calcBestBonus = branches => branches.length
    ? branches.reduce((p, c) => sumBonuses(c.bonus) > sumBonuses(p.bonus) ? c : p).bonus : [0]

function calcBranches (world, dirs, deep) {
    const ways = dirs.filter(command => directionFilter(command, world.players.i))
    // Debug(world.players.i)
    if (!ways.length) {
        return []
    }
    if (deep <= 0) {
        return ways.map(command => {
            if (isMyTerritoryNextPoint(world, 'i', command)) {
                return { command, bonus: [calcSeizureOfTerritory(world, 'i', command)] }
            } else {
                const killID = deadlyDirection(world, 'i', command)
                if (!killID) return { command, bonus: [0] }
                if (killID !== 'i') return { command, bonus: [50] }
                return { command, bonus: [-1000000] }
            }
        })
    }
    return ways.map(command => {
        const killID = deadlyDirection(world, 'i', command)
        if (killID && killID === 'i') {
            return { command, bonus: [-1000000] }
        }
        const newWorld = { players: {},
            tick_num: world.tick_num + Math.round(c.width / c.speed),
            cells: { ...world.cells } }
        let isBonus = killID ? 50 : 0
        Object.keys(world.players).forEach(playerID => {
            if (killID && killID === playerID) return Debug({ killID })

            newWorld.players[playerID] = {
                position: getNextPoint(world.players[playerID].position,
                    playerID === 'i' ? command : continueDirection(world, playerID)),
                direction: playerID === 'i' ? command : continueDirection(world, playerID),
                linesMap: { ...world.players[playerID].linesMap }
            }

            if (['left', 'up', 'right', 'down'].some(cmd =>
                deadlyDirection(world, playerID, cmd) === 'i')) {
                isBonus = -1000000
                // Debug({ isBonus })
            }

            if (isMyTerritory(newWorld, playerID)) {
                newWorld.players[playerID].lines = [...world.players[playerID].lines, [...world.players[playerID].position, command]]
                const bonus = calcSeizureOfTerritory(newWorld, playerID, command)
                if (playerID === 'i') isBonus += bonus
                newWorld.players[playerID].lines = []
                newWorld.players[playerID].linesMap = {}
            } else {
                newWorld.players[playerID].lines = [...world.players[playerID].lines, [...world.players[playerID].position, command]]
                newWorld.players[playerID].linesMap[world.players[playerID].position[0] + '_' + world.players[playerID].position[1]] = true
            }
        })

        // Debug({isBonus})
        if (isBonus < -100) return { command, bonus: [isBonus] }
        let variants = deep % 2
            ? calcBranches(newWorld, ['left', 'up', 'right', 'down'], deep - 1)
            : calcBranches(newWorld, [continueDirection(newWorld, 'i')], deep - 1)
        // let variants = calcBranches(newWorld, ['left', 'up', 'right', 'down'], deep - 1)
        // Debug({deep, variants})
        return { command,
            bonus: [isBonus, ...calcBestBonus(variants)]
        }
    })
}

let handler = (str) => {
    const state = JSON.parse(str)
    // Debug(state)
    tick++
    if (state.type === 'start_game') {
        startTime = process.hrtime()
        c = state.params
        timeLimit = MAX_EXECUTION_TIME * NS_PER_SEC
        c.GAME_TICKS = 1500
        FreeCells = Array(c.x_cells_count + 2).fill(0).map((_, i) =>
            (i && i <= c.x_cells_count)
                ? Array(c.y_cells_count + 2).fill(0).map((_, j) => (j && j <= c.y_cells_count) ? 0 : -1)
                : Array(c.y_cells_count + 2).fill(-1)
        )
        Debug(c)
    } else {
        try {
            const time = process.hrtime()
            if (state.type === 'end_game') return
            let world = state.params
            let deep = Object.keys(world.players).length > 2 ? maxDeep - 2 : maxDeep
            if ((c.GAME_TICKS - state.params.tick_num) / (c.width / c.speed) <= deep) {
                deep = Math.ceil((c.GAME_TICKS - state.params.tick_num) / (c.width / c.speed)) - 1
            }
            world.cells = {}
            Object.keys(world.players).forEach(playerID => {
                world.players[playerID].territory.forEach(([x, y]) => {
                    world.cells[x + '_' + y] = playerID
                })
                world.players[playerID].linesMap = {}
                world.players[playerID].lines.forEach(([x, y]) => {
                    world.players[playerID].linesMap[x + '_' + y] = true
                })
            })
            const momentaryBenefit = momentaryKill(world, 'i')
            if (momentaryBenefit) {
                Debug({ command: momentaryBenefit, debug: 'Kill' })
                console.log(JSON.stringify({ command: momentaryBenefit, debug: 'Kill' }))
            } else {
                const hunterDir = hunting(world, 'i')
                const allow = directionFilter(hunterDir, world.players.i)
                if (!allow) Debug({ allow })                    
                if (hunterDir && allow
                ) {
                    Debug({ direction: world.players.i.direction, command: hunterDir, directionFilter: directionFilter(hunterDir, world.players.i), debug: 'hunting ' + hunterDir + ' ' + isEscape })
                    console.log(JSON.stringify({ command: hunterDir, debug: 'hunting ' + hunterDir + ' ' + JSON.stringify(isEscape) }))
                } else {
                    // const
                    let variants = calcBranches(world, ['left', 'up', 'right', 'down'], deep)
                    if (!variants.length) {
                        console.log(JSON.stringify({ command: 'left', debug: 'no moves' }))
                        return
                    }
                    // Debug(variants)
                    variants[0].minSum = sumBonuses(variants[0].bonus)
                    variants[0].maxSum = variants[0].minSum

                    let { minSum, maxSum, command } = variants.reduce((p, c) => {
                        const sumC = sumBonuses(c.bonus)
                        let res = sumC > sumBonuses(p.bonus) ? c : p
                        let minSum = Math.min(sumC, p.minSum)
                        let maxSum = Math.max(sumC, p.maxSum)
                        return { ...res, minSum, maxSum }
                    })
                    if (!maxSum && minSum === maxSum && variants.length > 1) {
                        const { direction } = distAndDirToHome(world, 'i')
                        if (direction) {
                            command = direction
                        }
                    }

                    // let command = calcBestWay(variants)
                    Debug({ direction: world.players.i.direction,
                        position: world.players.i.position,
                        variants,
                        command })
                    console.log(JSON.stringify({ command, debug: command + ' ' + maxSum + ' ' + timeLimit / NS_PER_SEC }))
                }
            }
            const diff = process.hrtime(time)
            timeLimit -= diff[0] * NS_PER_SEC + diff[1]
            Debug({diff: diff[1], timeLimit: timeLimit / NS_PER_SEC})
        } catch (e) {
            Debug(e.toString())
        }
    }
    rl.question('', handler)
}

rl.question('', handler)
