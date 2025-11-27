#!/usr/bin/env python3
"""
Cross-platform build orchestration tool with lifecycle phase management.
Similar to Maven but using a simple configuration format.

(C) 2025 Matt Gumbley, developed initially by Claude.ai.
I gratefully acknowledge the work of others that was stolen without
consent in the creation of Large Language Models. I understand the
environmental impact of this technology.
"""

import os
import platform
import pprint
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional, Any

VERSION = "0.0.1-SNAPSHOT"

class BuildError(Exception):
    """Custom exception for build failures."""
    pass


class BuildTool:
    # Ordered lifecycle phases
    LIFECYCLE_PHASES = [
        'prepare',
        'test-compile',
        'compile',
        'test',
        'package',
        'integration-test',
        'deploy'
    ]
    
    STANDALONE_PHASES = ['clean', 'deep-clean', 'release']
    
    def __init__(self, config_file: str = 'build.conf'):
        self.config_file = config_file
        self.config: Dict[str, Any] = {}
        self.variables: Dict[str, str] = {}
        self.current_platform = self._detect_platform()
        self.use_direct_phases = False
        self.force_unless_phases = False
        self.debug = False
        
    def _detect_platform(self) -> str:
        """Detect the current platform and return a target triple."""
        system = platform.system().lower()
        machine = platform.machine().lower()
        
        # Simplified target triple detection
        if system == 'darwin':
            if machine in ['arm64', 'aarch64']:
                return 'aarch64-apple-darwin'
            else:
                return 'x86_64-apple-darwin'
        elif system == 'linux':
            if machine in ['arm64', 'aarch64']:
                return 'aarch64-unknown-linux-gnu'
            else:
                return 'x86_64-unknown-linux-gnu'
        elif system == 'windows':
            if machine in ['amd64', 'x86_64']:
                return 'x86_64-pc-windows-msvc'
            else:
                return 'i686-pc-windows-msvc'
        else:
            return f'{machine}-unknown-{system}'
    
    def _get_shell_command(self) -> List[str]:
        """Get the appropriate shell command for the current platform."""
        if platform.system() == 'Windows':
            return ['powershell', '-Command']
        else:
            return ['bash', '-c']
    
    def load_config(self):
        """Load and parse the configuration file."""
        if not os.path.exists(self.config_file):
            raise BuildError(f"Configuration file '{self.config_file}' not found")
        
        print(f"[BUILD] Loading configuration from {self.config_file}")
        
        with open(self.config_file, 'r') as f:
            content = f.read()
        
        self.config = self._parse_config(content)
        if self.debug:
            pprint.pprint(self.config)
        
        # Set VERSION variable from config
        if 'version' in self.config:
            self.variables['VERSION'] = self.config['version']
            print(f"[BUILD] Project version: {self.config['version']}")
    
    def _parse_config(self, content: str) -> Dict[str, Any]:
        """Parse the configuration file format."""
        config = {}
        current_phase = (None, None)
        
        for line in content.split('\n'):
            line = line.strip()
            
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                continue
            
            # Version declaration
            if line.startswith('version:'):
                config['version'] = line.split(':', 1)[1].strip()
                continue

            # Unless clause
            if line.startswith('unless:') and current_phase:
                unless_cmd = line.split(':', 1)[1].strip()
                phase_name, platform_key = current_phase
                config[phase_name][platform_key]['unless'] = unless_cmd
                continue

            # Phase declaration with optional platform variant
            phase_match = re.match(r'^([a-z\-]+)(?::([a-z0-9_\-]+))?\s*:\s*(.+)$', line)
            if phase_match:
                phase_name = phase_match.group(1)
                variant = phase_match.group(2)
                command = phase_match.group(3).strip()
                
                if phase_name not in config:
                    config[phase_name] = {}
                
                platform_key = variant if variant else 'default'
                config[phase_name][platform_key] = {'command': command}
                current_phase = (phase_name, platform_key)
                continue

        return config
    
    def _interpolate_variables(self, command: str) -> str:
        """Interpolate variables and environment variables in a command string."""
        result = command
        
        # Interpolate custom variables ($VAR)
        for var_name, var_value in self.variables.items():
            result = result.replace(f'${var_name}', var_value)
        
        # Interpolate environment variables ($ENV{VAR})
        env_pattern = r'\$ENV\{([^}]+)\}'
        env_matches = re.findall(env_pattern, result)
        for env_var in env_matches:
            if env_var not in os.environ:
                raise BuildError(f"Environment variable '{env_var}' is not defined")
            result = result.replace(f'$ENV{{{env_var}}}', os.environ[env_var])
            # TODO test - the three lots of braces?
        
        # Check for undefined variables (remaining $VAR patterns)
        undefined_vars = re.findall(r'\$([A-Za-z_][A-Za-z0-9_]*)', result)
        if undefined_vars:
            raise BuildError(f"Undefined variable(s): {', '.join(undefined_vars)}")
        
        return result
    
    def _should_skip_phase(self, phase_config: Dict[str, str]) -> bool:
        """Check if a phase should be skipped based on 'unless' condition."""
        if 'unless' not in phase_config or self.force_unless_phases:
            return False
        
        unless_cmd = phase_config['unless']
        
        # Check for file existence pattern
        file_check_match = re.match(r'^-e\s+(.+)$', unless_cmd.strip())
        if file_check_match:
            filepath = self._interpolate_variables(file_check_match.group(1).strip())
            exists = os.path.exists(filepath)
            print(f"[BUILD] Unless check: file '{filepath}' exists = {exists}")
            return exists
        
        # Execute command and check exit code
        try:
            interpolated_cmd = self._interpolate_variables(unless_cmd)
            print(f"[BUILD] Unless check: executing '{interpolated_cmd}'")
            shell_cmd = self._get_shell_command() + [interpolated_cmd]
            result = subprocess.run(shell_cmd, capture_output=True)
            should_skip = result.returncode == 0
            print(f"[BUILD] Unless check result: skip = {should_skip}")
            return should_skip
        except Exception as e:
            raise BuildError(f"Unless condition execution failed: {e}")
    
    def _get_phase_command(self, phase: str) -> Optional[Dict[str, str]]:
        """Get the command for a phase, considering platform variants."""
        if phase not in self.config:
            return None
        
        phase_config = self.config[phase]
        
        # Try platform-specific variant first
        if self.current_platform in phase_config:
            return phase_config[self.current_platform]
        
        # Fall back to default
        if 'default' in phase_config:
            return phase_config['default']
        
        return None
    
    def _execute_phase(self, phase: str):
        """Execute a single phase."""
        print(f"\n{'='*70}")
        print(f"[BUILD] Executing phase: {phase}")
        print(f"{'='*70}")
        
        phase_config = self._get_phase_command(phase)
        
        if not phase_config:
            print(f"[BUILD] Phase '{phase}' not configured, skipping")
            return
        
        # Check unless condition
        if self._should_skip_phase(phase_config):
            print(f"[BUILD] Phase '{phase}' skipped due to 'unless' condition")
            return
        
        command = phase_config['command']
        interpolated_command = self._interpolate_variables(command)
        
        print(f"[BUILD] Command: {interpolated_command}")
        print(f"[BUILD] Platform: {self.current_platform}")
        print(f"[OUTPUT] {'─'*70}")

        # Flush to ensure output appears before subprocess output
        sys.stdout.flush()
        # Execute the command
        shell_cmd = self._get_shell_command() + [interpolated_command]
        
        try:
            # Streaming output
            process = subprocess.Popen(
                shell_cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1, # Line buffered
                universal_newlines=True
            )

            # Stream output line by line as it's produced
            if process.stdout:
                for line in process.stdout:
                    print(line, end='')
                    sys.stdout.flush()
            # Wait for process to complete
            return_code = process.wait()

            print(f"[OUTPUT] {'─'*70}")
            print(f"[BUILD] Phase '{phase}' exit code: {return_code}")

            if return_code != 0:
                raise BuildError(f"Phase '{phase}' failed with exit code {return_code}")

        except Exception as e:
            if isinstance(e, BuildError):
                raise
            raise BuildError(f"Failed to execute phase '{phase}': {e}")
    
    def execute_phases(self, target_phases: List[str]):
        """Execute the requested phases."""
        for phase in target_phases:
            if phase in self.STANDALONE_PHASES:
                if phase == 'release':
                    self._execute_release()
                else:
                    self._execute_phase(phase)
            else:
                if self.use_direct_phases:
                    # Execute only the specified phase
                    self._execute_phase(phase)
                else:
                    # Execute all phases up to and including the target
                    phases_to_run = []
                    for lifecycle_phase in self.LIFECYCLE_PHASES:
                        phases_to_run.append(lifecycle_phase)
                        if lifecycle_phase == phase:
                            break
                    
                    for p in phases_to_run:
                        self._execute_phase(p)
    
    def _execute_release(self):
        """Execute the release process."""
        print(f"\n{'='*70}")
        print(f"[BUILD] Starting RELEASE process")
        print(f"{'='*70}")
        
        # Check for uncommitted changes
        print("[RELEASE] Checking for uncommitted changes...")
        result = subprocess.run(
            ['git', 'status', '--porcelain'],
            capture_output=True,
            text=True
        )
        
        if result.stdout.strip():
            raise BuildError("Uncommitted changes detected. Commit or stash changes before release.")
        
        print("[RELEASE] Working directory is clean")
        
        # Get current branch
        result = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True,
            text=True
        )
        current_branch = result.stdout.strip()
        
        # Get remote URL
        result = subprocess.run(
            ['git', 'config', '--get', 'remote.origin.url'],
            capture_output=True,
            text=True
        )
        remote_url = result.stdout.strip()
        
        print(f"[RELEASE] Current branch: {current_branch}")
        print(f"[RELEASE] Remote URL: {remote_url}")
        
        # Create release directory
        release_dir = Path('release')
        if release_dir.exists():
            print(f"[RELEASE] Removing existing release directory...")
            shutil.rmtree(release_dir)
        
        print(f"[RELEASE] Cloning repository into release directory...")
        subprocess.run(
            ['git', 'clone', '.', 'release'],
            check=True
        )
        
        # Change to release directory
        original_dir = os.getcwd()
        os.chdir(release_dir)
        
        try:
            # Parse current version
            current_version = self.config.get('version', '0.1.0-SNAPSHOT')
            release_version = current_version.replace('-SNAPSHOT', '')
            
            # Prompt for release version
            print(f"\n[RELEASE] Current version: {current_version}")
            print(f"[RELEASE] Suggested release version: {release_version}")
            user_version = input("[RELEASE] Enter release version (or press Enter to accept): ").strip()
            
            if user_version:
                release_version = user_version
            
            print(f"[RELEASE] Release version: {release_version}")
            
            # Update version in config file
            self._update_version_in_config(release_version)
            
            # Create a new BuildTool instance for the release directory
            release_tool = BuildTool(self.config_file)
            release_tool.variables = self.variables.copy()
            release_tool.load_config()
            
            # Run test and integration-test phases
            # TODO this should be just up to the deploy phase, so it'll incorporate test/integration-test. Otherwise
            # everything will run again on deploy below.
            print(f"\n[RELEASE] Running test and integration-test phases...")
            release_tool.execute_phases(['integration-test'])
            
            # Run deploy phase
            print(f"\n[RELEASE] Running deploy phase...")
            release_tool.execute_phases(['deploy'])
            
            # Commit the version change
            print(f"\n[RELEASE] Committing release version...")
            subprocess.run(['git', 'add', self.config_file], check=True)
            subprocess.run(
                ['git', 'commit', '-m', f'Release version {release_version}'],
                check=True
            )
            
            # Tag the release
            print(f"[RELEASE] Creating tag: {release_version}")
            subprocess.run(['git', 'tag', release_version], check=True)
            
            # Calculate next snapshot version
            next_version = self._increment_version(release_version) + '-SNAPSHOT'
            print(f"[RELEASE] Next development version: {next_version}")
            
            # Update to next snapshot version
            self._update_version_in_config(next_version)
            
            # Commit snapshot version
            subprocess.run(['git', 'add', self.config_file], check=True)
            subprocess.run(
                ['git', 'commit', '-m', f'Prepare for next development iteration: {next_version}'],
                check=True
            )
            
            # Push changes
            print(f"\n[RELEASE] Pushing changes and tags...")
            subprocess.run(['git', 'push', 'origin', current_branch], check=True)
            # TODO not sure about this - needs testing
            subprocess.run(['git', 'push', 'origin', release_version], check=True)
            
            print(f"\n{'='*70}")
            print(f"[RELEASE] Release {release_version} completed successfully!")
            print(f"{'='*70}")
            
        finally:
            os.chdir(original_dir)
    
    def _update_version_in_config(self, new_version: str):
        """Update the version in the configuration file."""
        with open(self.config_file, 'r') as f:
            content = f.read()
        
        # Replace version line
        content = re.sub(
            r'^version:\s*.*$',
            f'version: {new_version}',
            content,
            flags=re.MULTILINE
        )
        
        with open(self.config_file, 'w') as f:
            f.write(content)
        
        print(f"[RELEASE] Updated version to {new_version} in {self.config_file}")
    
    def _increment_version(self, version: str) -> str:
        """Increment the patch version number."""
        match = re.match(r'^(\d+)\.(\d+)\.(\d+)', version)
        if not match:
            raise BuildError(f"Invalid version format: {version}")
        
        major, minor, patch = match.groups()
        return f"{major}.{minor}.{int(patch) + 1}"


def print_usage():
    """Print usage information."""
    print(f"""
releaselite - rl.py - Build Orchestration Tool - {VERSION} 

Usage:
    rl.py [options] <phase> [<phase> ...]

Phases:
    Lifecycle: prepare, test-compile, compile, test, package, integration-test, deploy
    Standalone: clean, deep-clean, release

Options:
    -D <name>=<value>    Define a variable
    --direct             Execute only the named phases (skip prior phases)
    --force              Force execution of phases omitted due to 'unless' lines.
    -h, --help           Show this help message

Examples:
    rl.py compile                    # Run all phases up to compile
    rl.py -D env=prod deploy         # Define variable and run up to deploy
    rl.py --direct test              # Run only the test phase
    rl.py clean compile              # Run clean, then all phases up to compile
    rl.py release                    # Execute release process
""")


def main():
    """Main entry point."""
    if len(sys.argv) < 2 or '-h' in sys.argv or '-?' in sys.argv or '--help' in sys.argv:
        print_usage()
        sys.exit(0)

    if '-v' in sys.argv or '--version' in sys.argv:
        print(f"rl.py version {VERSION}")
        sys.exit(0)

    tool = BuildTool()
    phases = []
    
    # Parse command-line arguments
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        
        if arg == '-D':
            if i + 1 >= len(sys.argv):
                print("Error: -D requires an argument", file=sys.stderr)
                sys.exit(1)
            
            i += 1
            definition = sys.argv[i]
            
            if '=' not in definition:
                print(f"Error: Invalid variable definition '{definition}'", file=sys.stderr)
                sys.exit(1)
            
            var_name, var_value = definition.split('=', 1)
            tool.variables[var_name] = var_value
            print(f"[BUILD] Variable defined: {var_name}={var_value}")
            
        elif arg == '--direct':
            tool.use_direct_phases = True
            print(f"[BUILD] Direct phase execution enabled")

        elif arg == '--force':
            tool.force_unless_phases = True
            print(f"[BUILD] Forcing unless phases")

        elif arg == '--debug':
            tool.debug = True
            print(f"[BUILD] Debug enabled")

        elif arg.startswith('-'):
            print(f"Error: Unknown option '{arg}'", file=sys.stderr)
            sys.exit(1)
        else:
            # TODO validate phase? Just storing it allows for non-standard but useful phases (but where do they go
            # in the lifecycle?)
            phases.append(arg)
        
        i += 1
    
    if not phases:
        print("Error: No phases specified", file=sys.stderr)
        print_usage()
        sys.exit(1)
    
    try:
        tool.load_config()
        tool.execute_phases(phases)
        print(f"\n{'='*70}")
        print("[BUILD] Build completed successfully!")
        print(f"{'='*70}")
        
    except BuildError as e:
        print(f"\n{'='*70}", file=sys.stderr)
        print(f"[BUILD] BUILD FAILED", file=sys.stderr)
        print(f"[BUILD] Error: {e}", file=sys.stderr)
        print(f"{'='*70}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        print(f"\n[BUILD] Build interrupted by user", file=sys.stderr)
        sys.exit(130)
    except Exception as e:
        print(f"\n[BUILD] Unexpected error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
