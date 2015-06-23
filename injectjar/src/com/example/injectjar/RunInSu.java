package com.example.injectjar;

public class RunInSu {

	public static void main(String[] args) {
		Runtime.getRuntime().load(args[1]);
	}

}
