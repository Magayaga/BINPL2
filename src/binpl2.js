/*
    BINPL (v2.0 / April 7, 2025)
    Copyright (c) 2025 Cyril John Magayaga
*/

const fs = require('fs');

// Helper function to trim both leading and trailing whitespace
function trim(str) {
    return str.trim();
}

// Convert binary string (with whitespace allowed) to integer
function binaryToInt(bin) {
    const cleaned = [...bin]
        .filter(c => !/\s/.test(c))
        .join('');

    for (const c of cleaned) {
        if (c !== '0' && c !== '1') {
            console.error(`Invalid binary digit: ${c}`);
            return -1;
        }
    }

    return parseInt(cleaned, 2);
}

function main() {
    const args = process.argv.slice(2);

    if (args.length < 1) {
        console.error(`Usage: node binpl2.js <BINPL2 source file>`);
        process.exit(1);
    }

    let content;
    try {
        content = fs.readFileSync(args[0], 'utf-8');
    } catch (err) {
        console.error("Error opening file:", err.message);
        process.exit(1);
    }

    const lines = content.split('\n');
    let inProgram = false;
    let foundBegin = false;
    let foundEnd = false;
    let programName = "";

    for (let line of lines) {
        line = trim(line.replace(/\r/g, ''));

        // Remove inline comments ";;"
        const commentIndex = line.indexOf(';;');
        if (commentIndex !== -1) {
            line = trim(line.slice(0, commentIndex));
        }

        if (line === '') continue;

        if (line.startsWith('::BEGIN') || line.startsWith('::START')) {
            if (inProgram) {
                console.error("Error: Nested start marker encountered.");
                process.exit(1);
            }
            inProgram = true;
            foundBegin = true;

            const parts = line.split(/\s+/);
            if (parts.length > 1) {
                programName = parts[1];
                console.log(`Starting program: ${programName}`);
            }
            continue;
        }

        if (line === '::END') {
            if (!inProgram) {
                console.error("Error: ::END encountered without a matching start marker.");
                process.exit(1);
            }
            inProgram = false;
            foundEnd = true;
            break;
        }

        if (inProgram) {
            const match = line.match(/^(\w+)\s+(.*)$/);
            if (!match) {
                console.error(`Invalid instruction format: ${line}`);
                continue;
            }

            const [_, type, value] = match;
            let num = 0;

            if (type === 'Octa') {
                num = parseInt(value, 8);
            } else if (type === 'Deci') {
                num = parseInt(value, 10);
            } else if (type === 'Hexa') {
                if (!/^[0-9a-fA-F]+$/.test(value)) {
                    console.error(`Invalid hexadecimal digit in value: ${value}`);
                    continue;
                }
                num = parseInt(value, 16);
            } else if (type === 'Bina') {
                num = binaryToInt(value);
                if (num < 0) continue;
            } else {
                console.error(`Unknown numeral system: ${type}`);
                continue;
            }

            process.stdout.write(String.fromCharCode(num));
        }
    }

    if (!foundBegin) {
        console.error("Error: Missing start marker (::BEGIN or ::START).");
        process.exit(1);
    }

    if (!foundEnd) {
        console.error("Error: Missing ::END marker.");
        process.exit(1);
    }
}

main();
